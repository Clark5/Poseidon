// -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*-
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// NS3 - Layer 4 Protocol base class
// George F. Riley, Georgia Tech, Spring 2007

#include "ip-l4-protocol.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (IpL4Protocol);

TypeId
IpL4Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IpL4Protocol")
    .SetParent<Object> ()
    .AddAttribute ("ProtocolNumber", "The Ip protocol number.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&IpL4Protocol::GetProtocolNumber),
                   MakeUintegerChecker<int> ())
  ;
  return tid;
}

IpL4Protocol::~IpL4Protocol ()
{
}

void
IpL4Protocol::ReceiveIcmp (Ipv4Address icmpSource, uint8_t icmpTtl,
                           uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo,
                           Ipv4Address payloadSource,Ipv4Address payloadDestination,
                           const uint8_t payload[8])
{}
void
IpL4Protocol::ReceiveIcmp (Ipv6Address icmpSource, uint8_t icmpTtl,
                           uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo,
                           Ipv6Address payloadSource, Ipv6Address payloadDestination,
                           const uint8_t payload[8])
{}

} //namespace ns3
