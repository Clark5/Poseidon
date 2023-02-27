/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include "tcp-socket.h"

NS_LOG_COMPONENT_DEFINE ("TcpSocket");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpSocket);

const char* const TcpSocket::TcpStateName[LAST_STATE] = { "CLOSED", "LISTEN", "SYN_SENT", "SYN_RCVD", "ESTABLISHED", "CLOSE_WAIT", "LAST_ACK", "FIN_WAIT_1", "FIN_WAIT_2", "CLOSING", "TIME_WAIT" };

TypeId
TcpSocket::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpSocket")
    .SetParent<Socket> ()
    .AddAttribute ("SndBufSize",
                   "TcpSocket maximum transmit buffer size (bytes)",
                   UintegerValue (4194304), // 4M
                   MakeUintegerAccessor (&TcpSocket::GetSndBufSize,
                                         &TcpSocket::SetSndBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RcvBufSize",
                   "TcpSocket maximum receive buffer size (bytes)",
				   UintegerValue(4194304), // 4M
                   MakeUintegerAccessor (&TcpSocket::GetRcvBufSize,
                                         &TcpSocket::SetRcvBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SegmentSize",
                   "TCP maximum segment size in bytes (may be adjusted based on MTU discovery)",
                   UintegerValue (988),
                   MakeUintegerAccessor (&TcpSocket::GetSegSize,
                                         &TcpSocket::SetSegSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SlowStartThreshold",
                   "TCP slow start threshold (bytes)",
                   UintegerValue (0xffff),
                   MakeUintegerAccessor (&TcpSocket::GetSSThresh,
                                         &TcpSocket::SetSSThresh),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("InitialCwnd",
                   "TCP initial congestion window size (segments)",
                   UintegerValue (1),
                   MakeUintegerAccessor (&TcpSocket::GetInitialCwnd,
                                         &TcpSocket::SetInitialCwnd),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ConnTimeout",
                   "TCP retransmission timeout when opening connection (seconds)",
                   TimeValue (Seconds (3)),
                   MakeTimeAccessor (&TcpSocket::GetConnTimeout,
                                     &TcpSocket::SetConnTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("ConnCount",
                   "Number of connection attempts (SYN retransmissions) before returning failure",
                   UintegerValue (6),
                   MakeUintegerAccessor (&TcpSocket::GetConnCount,
                                         &TcpSocket::SetConnCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DelAckTimeout",
                   "Timeout value for TCP delayed acks, in seconds",
                   TimeValue (Seconds (0.2)),
                   MakeTimeAccessor (&TcpSocket::GetDelAckTimeout,
                                     &TcpSocket::SetDelAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("DelAckCount",
                   "Number of packets to wait before sending a TCP ack",
                   UintegerValue (0),
                   MakeUintegerAccessor (&TcpSocket::GetDelAckMaxCount,
                                         &TcpSocket::SetDelAckMaxCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TcpNoDelay", "Set to true to disable Nagle's algorithm",
                   BooleanValue (true),
                   MakeBooleanAccessor (&TcpSocket::GetTcpNoDelay,
                                        &TcpSocket::SetTcpNoDelay),
                   MakeBooleanChecker ())
    .AddAttribute ("PersistTimeout",
                   "Persist timeout to probe for rx window",
                   TimeValue (Seconds (6)),
                   MakeTimeAccessor (&TcpSocket::GetPersistTimeout,
                                     &TcpSocket::SetPersistTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("ECN",
                   "ECN capability of TCP socket",
                   BooleanValue (true),
                   MakeBooleanAccessor (&TcpSocket::GetEcnCap,
                                        &TcpSocket::SetEcnCap),
                   MakeBooleanChecker ())
  ;
  return tid;
}

TcpSocket::TcpSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

TcpSocket::~TcpSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

} // namespace ns3
