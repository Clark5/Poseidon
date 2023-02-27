/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 */

#include "ipv4-route.h"
#include "ns3/net-device.h"
#include "ns3/assert.h"

namespace ns3 {

Ipv4Route::Ipv4Route ()
{
}

void
Ipv4Route::SetDestination (Ipv4Address dest)
{
  m_dest = dest;
}

Ipv4Address
Ipv4Route::GetDestination (void) const
{
  return m_dest;
}

void
Ipv4Route::SetSource (Ipv4Address src)
{
  m_source = src;
}

Ipv4Address
Ipv4Route::GetSource (void) const
{
  return m_source;
}

void
Ipv4Route::SetGateway (Ipv4Address gw)
{
  m_gateway = gw;
}

Ipv4Address
Ipv4Route::GetGateway (void) const
{
  return m_gateway;
}

void
Ipv4Route::SetOutputDevice (Ptr<NetDevice> outputDevice)
{
  m_outputDevice = outputDevice;
}

Ptr<NetDevice>
Ipv4Route::GetOutputDevice (void) const
{
  return m_outputDevice;
}

std::ostream& operator<< (std::ostream& os, Ipv4Route const& route)
{
  os << "source=" << route.GetSource () << " dest="<< route.GetDestination () <<" gw=" << route.GetGateway ();
  return os;
}

Ipv4MulticastRoute::Ipv4MulticastRoute ()
{
  m_ttls.clear ();
}

void 
Ipv4MulticastRoute::SetGroup (const Ipv4Address group)
{
  m_group = group;
}

Ipv4Address 
Ipv4MulticastRoute::GetGroup (void) const
{
  return m_group;
}

void 
Ipv4MulticastRoute::SetOrigin (const Ipv4Address origin)
{
  m_origin = origin;
}

Ipv4Address 
Ipv4MulticastRoute::GetOrigin (void) const
{
  return m_origin;
}

void 
Ipv4MulticastRoute::SetParent (uint32_t parent)
{
  m_parent = parent;
}

uint32_t 
Ipv4MulticastRoute::GetParent (void) const
{
  return m_parent;
}

void 
Ipv4MulticastRoute::SetOutputTtl (uint32_t oif, uint32_t ttl)
{
  if (ttl >= MAX_TTL)
    {
      // This TTL value effectively disables the interface
      std::map<uint32_t, uint32_t>::iterator iter;
      iter = m_ttls.find (oif);
      if (iter != m_ttls.end ())
        {
          m_ttls.erase (iter);
        }
    }
  else
    {
      m_ttls[oif] = ttl;
    }
}

uint32_t
Ipv4MulticastRoute::GetOutputTtl (uint32_t oif)
{
  // We keep this interface around for compatibility (for now)
  std::map<uint32_t, uint32_t>::const_iterator iter = m_ttls.find (oif);
  if (iter == m_ttls.end ())
    return((uint32_t)MAX_TTL);
  return(iter->second);
}

std::map<uint32_t, uint32_t>
Ipv4MulticastRoute::GetOutputTtlMap () const
{
  return(m_ttls);
}

} // namespace ns3
