/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
#ifndef CONSTANT_VELOCITY_HELPER_H
#define CONSTANT_VELOCITY_HELPER_H

#include "ns3/nstime.h"
#include "ns3/vector.h"
#include "ns3/box.h"

namespace ns3 {

class Rectangle;

/**
 * \ingroup mobility
 *
 * \brief Utility class used to move node with constant velocity.
 */
class ConstantVelocityHelper
{
public:
  ConstantVelocityHelper ();
  ConstantVelocityHelper (const Vector &position);
  ConstantVelocityHelper (const Vector &position,
                          const Vector &vel);

  void SetPosition (const Vector &position);
  Vector GetCurrentPosition (void) const;
  Vector GetVelocity (void) const;
  void SetVelocity (const Vector &vel);
  void Pause (void);
  void Unpause (void);

  void UpdateWithBounds (const Rectangle &rectangle) const;
  void UpdateWithBounds (const Box &bounds) const;
  void Update (void) const;
private:
  mutable Time m_lastUpdate;
  mutable Vector m_position;
  Vector m_velocity;
  bool m_paused;
};

} // namespace ns3

#endif /* CONSTANT_VELOCITY_HELPER_H */
