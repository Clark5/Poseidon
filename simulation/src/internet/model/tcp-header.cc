/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
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
 * Author: Raj Bhattacharjea <raj.b@gatech.edu>
 */

#include <stdint.h>
#include <iostream>
#include "tcp-header.h"
#include "tcp-option-no-op.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpHeader);

TcpHeader::TcpHeader ()
  : m_sourcePort (0),
    m_destinationPort (0),
    m_sequenceNumber (0),
    m_ackNumber (0),
    m_length (5),
    m_flags (0),
    m_windowSize (0xffff),
    m_urgentPointer (0),
    m_calcChecksum (false),
    m_goodChecksum (true)
{
}

TcpHeader::~TcpHeader ()
{
}

void
TcpHeader::EnableChecksums (void)
{
  m_calcChecksum = true;
}

void TcpHeader::SetSourcePort (uint16_t port)
{
  m_sourcePort = port;
}
void TcpHeader::SetDestinationPort (uint16_t port)
{
  m_destinationPort = port;
}
void TcpHeader::SetSequenceNumber (SequenceNumber32 sequenceNumber)
{
  m_sequenceNumber = sequenceNumber;
}
void TcpHeader::SetAckNumber (SequenceNumber32 ackNumber)
{
  m_ackNumber = ackNumber;
}
void TcpHeader::SetLength (uint8_t length)
{
  m_length = length;
}
void TcpHeader::SetFlags (uint16_t flags)
{
  m_flags = flags;
}
void TcpHeader::SetWindowSize (uint16_t windowSize)
{
  m_windowSize = windowSize;
}
void TcpHeader::SetUrgentPointer (uint16_t urgentPointer)
{
  m_urgentPointer = urgentPointer;
}

uint16_t TcpHeader::GetSourcePort () const
{
  return m_sourcePort;
}
uint16_t TcpHeader::GetDestinationPort () const
{
  return m_destinationPort;
}
SequenceNumber32 TcpHeader::GetSequenceNumber () const
{
  return m_sequenceNumber;
}
SequenceNumber32 TcpHeader::GetAckNumber () const
{
  return m_ackNumber;
}
uint8_t  TcpHeader::GetLength () const
{
  return m_length;
}
uint16_t  TcpHeader::GetFlags () const
{
  return m_flags;
}
uint16_t TcpHeader::GetWindowSize () const
{
  return m_windowSize;
}
uint16_t TcpHeader::GetUrgentPointer () const
{
  return m_urgentPointer;
}

void
TcpHeader::InitializeChecksum (Ipv4Address source,
                               Ipv4Address destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

void
TcpHeader::InitializeChecksum (Ipv6Address source,
                               Ipv6Address destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

void
TcpHeader::InitializeChecksum (Address source,
                               Address destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

uint16_t
TcpHeader::CalculateHeaderChecksum (uint16_t size) const
{
  /* Buffer size must be at least as large as the largest IP pseudo-header */
  /* [per RFC2460, but without consideration for IPv6 extension hdrs]      */
  /* Src address            16 bytes (more generally, Address::MAX_SIZE)   */
  /* Dst address            16 bytes (more generally, Address::MAX_SIZE)   */
  /* Upper layer pkt len    4 bytes                                        */
  /* Zero                   3 bytes                                        */
  /* Next header            1 byte                                         */

  uint32_t maxHdrSz = (2 * Address::MAX_SIZE) + 8;
  Buffer buf = Buffer (maxHdrSz);
  buf.AddAtStart (maxHdrSz);
  Buffer::Iterator it = buf.Begin ();
  uint32_t hdrSize = 0;

  WriteTo (it, m_source);
  WriteTo (it, m_destination);
  if (Ipv4Address::IsMatchingType(m_source))
    {
      it.WriteU8 (0); /* protocol */
      it.WriteU8 (m_protocol); /* protocol */
      it.WriteU8 (size >> 8); /* length */
      it.WriteU8 (size & 0xff); /* length */
      hdrSize = 12;
    }
  else
    {
      it.WriteU16 (0);
      it.WriteU8 (size >> 8); /* length */
      it.WriteU8 (size & 0xff); /* length */
      it.WriteU16 (0);
      it.WriteU8 (0);
      it.WriteU8 (m_protocol); /* protocol */
      hdrSize = 40;
    }

  it = buf.Begin ();
  /* we don't CompleteChecksum ( ~ ) now */
  return ~(it.CalculateIpChecksum (hdrSize));
}

bool
TcpHeader::IsChecksumOk (void) const
{
  return m_goodChecksum;
}

TypeId
TcpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpHeader")
    .SetParent<Header> ()
    .AddConstructor<TcpHeader> ()
  ;
  return tid;
}
TypeId
TcpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void TcpHeader::Print (std::ostream &os)  const
{
	/*
  os << m_sourcePort << " > " << m_destinationPort;
  if(m_flags!=0)
    {
      os<<" [";
      if((m_flags & FIN) != 0)
        {
          os<<" FIN ";
        }
      if((m_flags & SYN) != 0)
        {
          os<<" SYN ";
        }
      if((m_flags & RST) != 0)
        {
          os<<" RST ";
        }
      if((m_flags & PSH) != 0)
        {
          os<<" PSH ";
        }
      if((m_flags & ACK) != 0)
        {
          os<<" ACK ";
        }
      if((m_flags & URG) != 0)
        {
          os<<" URG ";
        }
      if((m_flags & ECE) != 0)
        {
          os<<" ECE ";
        }
      if((m_flags & CWR) != 0)
        {
          os<<" CWR ";
        }
      if((m_flags & NS) != 0)
        {
          os<<" NS ";
        }
      os<<"]";
    }
  os<<" Seq="<<m_sequenceNumber<<" Ack="<<m_ackNumber<<" Win="<<m_windowSize;
  if (!m_options.empty ())
    {
      os << "Options:" << std::endl;
    }
  for (std::list<Ptr<TcpOption> >::const_iterator op = m_options.begin (); op != m_options.end (); ++op)
    {
      os<<" - "<< (*op)->Str () <<" - ";
    }
	*/
	os << "t " << m_destinationPort << ' ' << m_sequenceNumber  << " 1";
}
uint32_t TcpHeader::GetSerializedSize (void)  const
{
  return 4*GetLength ();
}
void TcpHeader::Serialize (Buffer::Iterator start)  const
{
  Buffer::Iterator i = start;
  i.WriteHtonU16 (m_sourcePort);
  i.WriteHtonU16 (m_destinationPort);
  i.WriteHtonU32 (m_sequenceNumber.GetValue ());
  i.WriteHtonU32 (m_ackNumber.GetValue ());
  i.WriteHtonU16 (GetLength () << 12 | m_flags); //reserved bits are all zero
  i.WriteHtonU16 (m_windowSize);
  i.WriteHtonU16 (0);
  i.WriteHtonU16 (m_urgentPointer);

  // Serialize options if they exists
  unsigned optionLen = 0;
  for (std::list<Ptr<TcpOption> >::const_iterator op = m_options.begin (); op != m_options.end (); ++op)
    {
      optionLen += (*op)->GetSerializedSize ();
      (*op)->Serialize(i);
    };
  while (optionLen % 4)
    {
      TcpOptionNoOp noOp;
      optionLen += noOp.GetSerializedSize ();
      noOp.Serialize(i);
    };

  // Make checksum
  if(m_calcChecksum)
    {
      uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (start.GetSize (), headerChecksum);

      i = start;
      i.Next (16);
      i.WriteU16 (checksum);
    }
}
uint32_t TcpHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_sourcePort = i.ReadNtohU16 ();
  m_destinationPort = i.ReadNtohU16 ();
  m_sequenceNumber = i.ReadNtohU32 ();
  m_ackNumber = i.ReadNtohU32 ();
  uint16_t field = i.ReadNtohU16 ();
  m_flags = field & 0x1FF;
  m_length = field>>12;
  m_windowSize = i.ReadNtohU16 ();
  i.Next (2);
  m_urgentPointer = i.ReadNtohU16 ();

  // Deserialize options if they exist
  m_options.clear ();
  uint32_t optionLen = (m_length - 5) * 4;
  while (optionLen)
    {
      Ptr<TcpOption> op = TcpOption::Deserialize (i, optionLen);
      if (!!op && op->GetKind () != TcpOption::NO_OP &&
            op->GetKind () != TcpOption::END_OF_OPTION_LIST)
        {
          m_options.push_back (op);
        }
    };

  // Do checksum
  if(m_calcChecksum)
    {
      uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (start.GetSize (), headerChecksum);
      m_goodChecksum = (checksum == 0);
    }

  return GetSerializedSize ();
}

void TcpHeader::AppendOption (Ptr<TcpOption> option)
{
  if (option->GetKind () != 0)
    {
      m_options.push_back (option);
    }
  // Update length
  unsigned totalLen = 0;
  for (std::list<Ptr<TcpOption> >::iterator i = m_options.begin (); i != m_options.end (); ++i)
  {
    totalLen += (*i)->GetSerializedSize ();
  };
  NS_ASSERT(totalLen <= 40);

  if (totalLen % 4 != 0)
    totalLen += (4 - (totalLen % 4));

  totalLen += 20;
  m_length = totalLen / 4;
}

Ptr<TcpOption> TcpHeader::GetOption (TcpOption::Kind kind) const
{
  for (std::list<Ptr<TcpOption> >::const_iterator i = m_options.begin (); i != m_options.end () ; ++i)
    {
      if ((*i)->GetKind () == kind)
        {
          return (*i);
        }
    }
  return NULL;
}

} // namespace ns3
