/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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

#include "icmpv4.h"
#include "ns3/packet.h"

namespace ns3 {

/********************************************************
 *        Icmpv4Header
 ********************************************************/

NS_OBJECT_ENSURE_REGISTERED (Icmpv4Header);

TypeId 
Icmpv4Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Icmpv4Header")
    .SetParent<Header> ()
    .AddConstructor<Icmpv4Header> ()
  ;
  return tid;
}
Icmpv4Header::Icmpv4Header ()
  : m_type (0),
    m_code (0),
    m_calcChecksum (false)
{
}
Icmpv4Header::~Icmpv4Header ()
{
}
void
Icmpv4Header::EnableChecksum (void)
{
  m_calcChecksum = true;
}
TypeId
Icmpv4Header::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
Icmpv4Header::GetSerializedSize (void) const
{
  return 4;
}
void
Icmpv4Header::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_type);
  i.WriteU8 (m_code);
  i.WriteHtonU16 (0);
  if (m_calcChecksum)
    {
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (i.GetSize ());
      i = start;
      i.Next (2);
      i.WriteU16 (checksum);
    }

}
uint32_t 
Icmpv4Header::Deserialize (Buffer::Iterator start)
{
  m_type = start.ReadU8 ();
  m_code = start.ReadU8 ();
  start.Next (2); // uint16_t checksum = start.ReadNtohU16 ();
  return 4;
}
void 
Icmpv4Header::Print (std::ostream &os) const
{
  os << "type=" << (uint32_t)m_type << ", code=" << (uint32_t)m_code;
}

void 
Icmpv4Header::SetType (uint8_t type)
{
  m_type = type;
}
void 
Icmpv4Header::SetCode (uint8_t code)
{
  m_code = code;
}
uint8_t 
Icmpv4Header::GetType (void) const
{
  return m_type;
}
uint8_t 
Icmpv4Header::GetCode (void) const
{
  return m_code;
}

/********************************************************
 *        Icmpv4Echo
 ********************************************************/

NS_OBJECT_ENSURE_REGISTERED (Icmpv4Echo);

void 
Icmpv4Echo::SetIdentifier (uint16_t id)
{
  m_identifier = id;
}
void 
Icmpv4Echo::SetSequenceNumber (uint16_t seq)
{
  m_sequence = seq;
}
void 
Icmpv4Echo::SetData (Ptr<const Packet> data)
{
  uint32_t size = data->GetSize ();
  //
  // All kinds of optimizations are possible, but let's not get carried away
  // since this is probably a very uncommon thing in the big picture.
  //
  // N.B. Zero is a legal size for the alloc below even though a hardcoded zero
  // would result in  warning.
  //
  if (size != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t[size];
      m_dataSize = size;
    }
  data->CopyData (m_data, size);
}
uint16_t 
Icmpv4Echo::GetIdentifier (void) const
{
  return m_identifier;
}
uint16_t 
Icmpv4Echo::GetSequenceNumber (void) const
{
  return m_sequence;
}
uint32_t
Icmpv4Echo::GetDataSize (void) const
{
  return m_dataSize;
}
uint32_t
Icmpv4Echo::GetData (uint8_t payload[]) const
{
  memcpy (payload, m_data, m_dataSize);
  return m_dataSize;
}
TypeId 
Icmpv4Echo::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Icmpv4Echo")
    .SetParent<Header> ()
    .AddConstructor<Icmpv4Echo> ()
  ;
  return tid;
}
Icmpv4Echo::Icmpv4Echo ()
  : m_identifier (0),
    m_sequence (0),
    m_dataSize (0)
{
  //
  // After construction, m_data is always valid until destruction.  This is true
  // even if m_dataSize is zero.
  //
  m_data = new uint8_t[m_dataSize];
}
Icmpv4Echo::~Icmpv4Echo ()
{
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}
TypeId 
Icmpv4Echo::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t 
Icmpv4Echo::GetSerializedSize (void) const
{
  return 4 + m_dataSize;
}
void 
Icmpv4Echo::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_identifier);
  start.WriteHtonU16 (m_sequence);
  start.Write (m_data, m_dataSize);
}
uint32_t 
Icmpv4Echo::Deserialize (Buffer::Iterator start)
{
  m_identifier = start.ReadNtohU16 ();
  m_sequence = start.ReadNtohU16 ();
  NS_ASSERT (start.GetSize () >= 4);
  uint32_t size = start.GetSize () - 4;
  if (size != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t[size];
      m_dataSize = size;
    }
  start.Read (m_data, m_dataSize);
  return m_dataSize;
}
void 
Icmpv4Echo::Print (std::ostream &os) const
{
  os << "identifier=" << m_identifier << ", sequence="  << m_sequence;
}


/********************************************************
 *        Icmpv4DestinationUnreachable
 ********************************************************/

NS_OBJECT_ENSURE_REGISTERED (Icmpv4DestinationUnreachable);

TypeId 
Icmpv4DestinationUnreachable::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Icmpv4DestinationUnreachable")
    .SetParent<Header> ()
    .AddConstructor<Icmpv4DestinationUnreachable> ()
  ;
  return tid;
}
Icmpv4DestinationUnreachable::Icmpv4DestinationUnreachable ()
{
  // make sure that thing is initialized to get initialized bytes
  // when the ip payload's size is smaller than 8 bytes.
  for (uint8_t j = 0; j < 8; j++)
    {
      m_data[j] = 0;
    }
}

void 
Icmpv4DestinationUnreachable::SetNextHopMtu (uint16_t mtu)
{
  m_nextHopMtu = mtu;
}
uint16_t 
Icmpv4DestinationUnreachable::GetNextHopMtu (void) const
{
  return m_nextHopMtu;
}

void 
Icmpv4DestinationUnreachable::SetData (Ptr<const Packet> data)
{
  data->CopyData (m_data, 8);
}
void 
Icmpv4DestinationUnreachable::SetHeader (Ipv4Header header)
{
  m_header = header;
}
void 
Icmpv4DestinationUnreachable::GetData (uint8_t payload[8]) const
{
  memcpy (payload, m_data, 8);
}
Ipv4Header 
Icmpv4DestinationUnreachable::GetHeader (void) const
{
  return m_header;
}


Icmpv4DestinationUnreachable::~Icmpv4DestinationUnreachable ()
{
}
TypeId
Icmpv4DestinationUnreachable::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
Icmpv4DestinationUnreachable::GetSerializedSize (void) const
{
  return 4 + m_header.GetSerializedSize () + 8;
}
void
Icmpv4DestinationUnreachable::Serialize (Buffer::Iterator start) const
{
  start.WriteU16 (0);
  start.WriteHtonU16 (m_nextHopMtu);
  uint32_t size = m_header.GetSerializedSize ();
  m_header.Serialize (start);
  start.Next (size);
  start.Write (m_data, 8);
}

uint32_t 
Icmpv4DestinationUnreachable::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  i.Next (2);
  m_nextHopMtu = i.ReadNtohU16 ();
  uint32_t read = m_header.Deserialize (i);
  i.Next (read);
  for (uint8_t j = 0; j < 8; j++)
    {
      m_data[j] = i.ReadU8 ();
    }
  return i.GetDistanceFrom (start);
}
void 
Icmpv4DestinationUnreachable::Print (std::ostream &os) const
{
  m_header.Print (os);
  os << " org data=";
  for (uint8_t i = 0; i < 8; i++)
    {
      os << (uint32_t) m_data[i];
      if (i != 8)
        {
          os << " ";
        }
    }
}

/********************************************************
 *        Icmpv4TimeExceeded
 ********************************************************/

NS_OBJECT_ENSURE_REGISTERED (Icmpv4TimeExceeded);

TypeId 
Icmpv4TimeExceeded::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Icmpv4TimeExceeded")
    .SetParent<Header> ()
    .AddConstructor<Icmpv4TimeExceeded> ()
  ;
  return tid;
}
Icmpv4TimeExceeded::Icmpv4TimeExceeded ()
{
  // make sure that thing is initialized to get initialized bytes
  // when the ip payload's size is smaller than 8 bytes.
  for (uint8_t j = 0; j < 8; j++)
    {
      m_data[j] = 0;
    }
}


void 
Icmpv4TimeExceeded::SetData (Ptr<const Packet> data)
{
  data->CopyData (m_data, 8);
}
void 
Icmpv4TimeExceeded::SetHeader (Ipv4Header header)
{
  m_header = header;
}
void 
Icmpv4TimeExceeded::GetData (uint8_t payload[8]) const
{
  memcpy (payload, m_data, 8);
}
Ipv4Header 
Icmpv4TimeExceeded::GetHeader (void) const
{
  return m_header;
}


Icmpv4TimeExceeded::~Icmpv4TimeExceeded ()
{
}
TypeId
Icmpv4TimeExceeded::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
Icmpv4TimeExceeded::GetSerializedSize (void) const
{
  return 4 + m_header.GetSerializedSize () + 8;
}
void
Icmpv4TimeExceeded::Serialize (Buffer::Iterator start) const
{
  start.WriteU32 (0);
  uint32_t size = m_header.GetSerializedSize ();
  m_header.Serialize (start);
  start.Next (size);
  start.Write (m_data, 8);
}

uint32_t 
Icmpv4TimeExceeded::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  i.Next (4);
  uint32_t read = m_header.Deserialize (i);
  i.Next (read);
  for (uint8_t j = 0; j < 8; j++)
    {
      m_data[j] = i.ReadU8 ();
    }
  return i.GetDistanceFrom (start);
}
void 
Icmpv4TimeExceeded::Print (std::ostream &os) const
{
  m_header.Print (os);
  os << " org data=";
  for (uint8_t i = 0; i < 8; i++)
    {
      os << (uint32_t) m_data[i];
      if (i != 8)
        {
          os << " ";
        }
    }
}

} // namespace ns3
