/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Adrian Sai-wah Tam <adrian.sw.tam@gmail.com>
 */

#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include "tcp-tahoe.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("TcpTahoe");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpTahoe);

TypeId
TcpTahoe::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpTahoe")
    .SetParent<TcpSocketBase> ()
    .AddConstructor<TcpTahoe> ()
    .AddAttribute ("ReTxThreshold", "Threshold for fast retransmit",
                    UintegerValue (3),
                    MakeUintegerAccessor (&TcpTahoe::m_retxThresh),
                    MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("CongestionWindow",
                     "The TCP connection's congestion window",
                     MakeTraceSourceAccessor (&TcpTahoe::m_cWnd))
  ;
  return tid;
}

TcpTahoe::TcpTahoe (void)
  : m_initialCWnd (1),
    m_retxThresh (3),
    m_ssThreshLastChange (0)
{
  NS_LOG_FUNCTION (this);
}

TcpTahoe::TcpTahoe (const TcpTahoe& sock)
  : TcpSocketBase (sock),
    m_cWnd (sock.m_cWnd),
    m_ssThresh (sock.m_ssThresh),
    m_initialCWnd (sock.m_initialCWnd),
    m_retxThresh (sock.m_retxThresh),
    m_ssThreshLastChange (sock.m_ssThreshLastChange)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpTahoe::~TcpTahoe (void)
{
}

/** We initialize m_cWnd from this function, after attributes initialized */
int
TcpTahoe::Listen (void)
{
  NS_LOG_FUNCTION (this);
  InitializeCwnd ();
  return TcpSocketBase::Listen ();
}

/** We initialize m_cWnd from this function, after attributes initialized */
int
TcpTahoe::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);
  InitializeCwnd ();
  return TcpSocketBase::Connect (address);
}

/** Limit the size of in-flight data by cwnd and receiver's rxwin */
uint32_t
TcpTahoe::Window (void)
{
  NS_LOG_FUNCTION (this);
  return std::min (m_rWnd.Get (), m_cWnd.Get ());
}

Ptr<TcpSocketBase>
TcpTahoe::Fork (void)
{
  return CopyObject<TcpTahoe> (this);
}

/** New ACK (up to seqnum seq) received. Increase cwnd and call TcpSocketBase::NewAck() */
void
TcpTahoe::NewAck (SequenceNumber32 const& seq)
{
  NS_LOG_FUNCTION (this << seq);
  NS_LOG_LOGIC ("TcpTahoe receieved ACK for seq " << seq <<
                " cwnd " << m_cWnd <<
                " ssthresh " << m_ssThresh);
  if (m_cWnd < m_ssThresh)
    { // Slow start mode, add one segSize to cWnd. Default m_ssThresh is 65535. (RFC2001, sec.1)
      m_cWnd += m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << m_cWnd << " ssthresh " << m_ssThresh);
    }
  else
    { // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
      // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd
      double adder = static_cast<double> (m_segmentSize * m_segmentSize) / m_cWnd.Get ();
      adder = std::max (1.0, adder);
      m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << m_cWnd << " ssthresh " << m_ssThresh);
    }
  TcpSocketBase::NewAck (seq);           // Complete newAck processing
}

/** Cut down ssthresh upon triple dupack */
void
TcpTahoe::DupAck (const TcpHeader& t, uint32_t count)
{
  NS_LOG_FUNCTION (this << "t " << count);
  if (count == m_retxThresh)
    { // triple duplicate ack triggers fast retransmit (RFC2001, sec.3)
      NS_LOG_INFO ("Triple Dup Ack: old ssthresh " << m_ssThresh << " cwnd " << m_cWnd);
      // fast retransmit in Tahoe means triggering RTO earlier. Tx is restarted
      // from the highest ack and run slow start again.
      // (Fall & Floyd 1996, sec.1)
      m_ssThresh = std::max (static_cast<unsigned> (m_cWnd / 2), m_segmentSize * 2);  // Half ssthresh
      m_ssThreshLastChange = Simulator::Now ();
      m_cWnd = m_segmentSize; // Run slow start again
      m_nextTxSequence = m_txBuffer.HeadSequence (); // Restart from highest Ack
      NS_LOG_INFO ("Triple Dup Ack: new ssthresh " << m_ssThresh << " cwnd " << m_cWnd);
      NS_LOG_LOGIC ("Triple Dup Ack: retransmit missing segment at " << Simulator::Now ().GetSeconds ());
      DoRetransmit ();
    }
}

/** Retransmit timeout */
void TcpTahoe::Retransmit (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
  // If erroneous timeout in closed/timed-wait state, just return
  if (m_state == CLOSED || m_state == TIME_WAIT) return;
  // If all data are received (non-closing socket and nothing to send), just return
  if (m_state <= ESTABLISHED && m_txBuffer.HeadSequence () >= m_highTxMark) return;

  m_ssThresh = std::max (static_cast<unsigned> (m_cWnd / 2), m_segmentSize * 2);  // Half ssthresh
  m_ssThreshLastChange = Simulator::Now ();
  m_cWnd = m_segmentSize;                   // Set cwnd to 1 segSize (RFC2001, sec.2)
  m_nextTxSequence = m_txBuffer.HeadSequence (); // Restart from highest Ack
  m_rtt->IncreaseMultiplier ();             // Double the next RTO
  DoRetransmit ();                          // Retransmit the packet
}

void
TcpTahoe::SetSegSize (uint32_t size)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "TcpTahoe::SetSegSize() cannot change segment size after connection started.");
  m_segmentSize = size;
}

void
TcpTahoe::SetSSThresh (uint32_t threshold)
{
  m_ssThresh = threshold;
  m_ssThreshLastChange = Simulator::Now ();
}

uint32_t
TcpTahoe::GetSSThresh (void) const
{
  return m_ssThresh;
}

void
TcpTahoe::SetInitialCwnd (uint32_t cwnd)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "TcpTahoe::SetInitialCwnd() cannot change initial cwnd after connection started.");
  m_initialCWnd = cwnd;
}

uint32_t
TcpTahoe::GetInitialCwnd (void) const
{
  return m_initialCWnd;
}

void
TcpTahoe::InitializeCwnd (void)
{
  /*
   * Initialize congestion window, default to 1 MSS (RFC2001, sec.1) and must
   * not be larger than 2 MSS (RFC2581, sec.3.1). Both m_initiaCWnd and
   * m_segmentSize are set by the attribute system in ns3::TcpSocket.
   */
  m_cWnd = m_initialCWnd * m_segmentSize;
}

void
TcpTahoe::HalveCwnd(void)
{
  if (m_ssThreshLastChange + m_rtt->GetCurrentEstimate () < Simulator::Now())
    {
      m_ssThreshLastChange = Simulator::Now ();
      m_ssThresh = std::max ((uint32_t) (m_cWnd / 2), m_segmentSize * 2);  // Half ssthresh
    }
  double d = 1;
  if (m_deadline != 0)
    {
      double B = m_bytesToTx - m_rtt->GetBytesSent ();
      if (B <= 0)
        {
          d = 0.5;
        }
      else
        {
          double Tc = B * m_rtt->GetCurrentEstimate ().GetSeconds () / (3.0 * m_cWnd.Get() / 4.0);
          double D = (m_deadlineFinish.GetSeconds ()) - Simulator::Now().GetSeconds ();
          if (D <= 0)
            {
              d = 2.0;
            }
          else
            {
              d = Tc / D;
              d = std::min(d, 2.0);
              d = std::max(d, 0.5);
            }
        }
    }
  double alpha = m_DCTCP ? m_rtt->GetAlpha () : 1;
  double p = std::pow(alpha, d);
  double tmp = m_cWnd.Get() * (1 - p / 2);
  m_cWnd = std::max((uint32_t)tmp, m_segmentSize);
}

} // namespace ns3
