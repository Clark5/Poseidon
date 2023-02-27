/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/trailer.h"
#include "ethernet-trailer.h"

NS_LOG_COMPONENT_DEFINE ("EthernetTrailer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EthernetTrailer);

EthernetTrailer::EthernetTrailer ()
  : m_calcFcs (false),
    m_fcs (0)
{
}

void
EthernetTrailer::EnableFcs (bool enable)
{
  m_calcFcs = enable;
}

bool
EthernetTrailer::CheckFcs (Ptr<const Packet> p) const
{
  int len = p->GetSize ();
  uint8_t *buffer;
  uint32_t crc;

  if (!m_calcFcs)
    {
      return true;
    }

  buffer = new uint8_t[len];
  p->CopyData (buffer, len);
  crc = DoCalcFcs (buffer, len);
  delete[] buffer;
  return (m_fcs == crc);
}

void
EthernetTrailer::CalcFcs (Ptr<const Packet> p)
{
  int len = p->GetSize ();
  uint8_t *buffer;

  if (!m_calcFcs)
    {
      return;
    }

  buffer = new uint8_t[len];
  p->CopyData (buffer, len);
  m_fcs = DoCalcFcs (buffer, len);
  delete[] buffer;
}

void
EthernetTrailer::SetFcs (uint32_t fcs)
{
  m_fcs = fcs;
}

uint32_t
EthernetTrailer::GetFcs (void)
{
  return m_fcs;
}

uint32_t
EthernetTrailer::GetTrailerSize (void) const
{
  return GetSerializedSize ();
}

TypeId 
EthernetTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EthernetTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<EthernetTrailer> ()
  ;
  return tid;
}
TypeId 
EthernetTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void 
EthernetTrailer::Print (std::ostream &os) const
{
  os << "fcs=" << m_fcs;
}
uint32_t 
EthernetTrailer::GetSerializedSize (void) const
{
  return 4;
}

void
EthernetTrailer::Serialize (Buffer::Iterator end) const
{
  Buffer::Iterator i = end;
  i.Prev (GetSerializedSize ());

  i.WriteU32 (m_fcs);
}
uint32_t
EthernetTrailer::Deserialize (Buffer::Iterator end)
{
  Buffer::Iterator i = end;
  uint32_t size = GetSerializedSize ();
  i.Prev (size);

  m_fcs = i.ReadU32 ();

  return size;
}

// This code is copied from /lib/crc32.c in the linux kernel.
// It assumes little endian ordering.
uint32_t
EthernetTrailer::DoCalcFcs (uint8_t const *buffer, size_t len) const
{
  uint32_t crc = 0xffffffff;
  int i;

  while (len--)
    {
      crc ^= *buffer++;
      for (i = 0; i < 8; i++)
        {
          crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
        }
    }
  return ~crc;
}

} // namespace ns3
