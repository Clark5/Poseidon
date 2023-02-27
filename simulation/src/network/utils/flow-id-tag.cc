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
#include "flow-id-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (FlowIdTag);

TypeId 
FlowIdTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FlowIdTag")
    .SetParent<Tag> ()
    .AddConstructor<FlowIdTag> ()
  ;
  return tid;
}
TypeId 
FlowIdTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t 
FlowIdTag::GetSerializedSize (void) const
{
  return 4;
}
void 
FlowIdTag::Serialize (TagBuffer buf) const
{
  buf.WriteU32 (m_flowId);
}
void 
FlowIdTag::Deserialize (TagBuffer buf)
{
  m_flowId = buf.ReadU32 ();
}
void 
FlowIdTag::Print (std::ostream &os) const
{
  os << "FlowId=" << m_flowId;
}
FlowIdTag::FlowIdTag ()
  : Tag () 
{
}

FlowIdTag::FlowIdTag (uint32_t id)
  : Tag (),
    m_flowId (id)
{
}

void
FlowIdTag::SetFlowId (uint32_t id)
{
  m_flowId = id;
}
uint32_t
FlowIdTag::GetFlowId (void) const
{
  return m_flowId;
}

uint32_t 
FlowIdTag::AllocateFlowId (void)
{
  static uint32_t nextFlowId = 1;
  uint32_t flowId = nextFlowId;
  nextFlowId++;
  return flowId;
}

} // namespace ns3

