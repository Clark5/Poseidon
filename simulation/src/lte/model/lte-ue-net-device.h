/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef LTE_UE_NET_DEVICE_H
#define LTE_UE_NET_DEVICE_H

#include "lte-net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "lte-phy.h"
#include "lte-phy.h"


namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class LtePhy;
class LteUePhy;
class LteEnbNetDevice;
class LteUeMac;
class LteUeRrc;

/**
 * \ingroup lte
 * The LteUeNetDevice class implements the UE net device
 */
class LteUeNetDevice : public LteNetDevice
{

public:
  static TypeId GetTypeId (void);

  LteUeNetDevice (void);
  /**
   * \brief Create an UE net device
   * \param node
   * \param phy
   */
  LteUeNetDevice (Ptr<Node> node, Ptr<LteUePhy> phy, Ptr<LteUeMac> mac, Ptr<LteUeRrc> rrc);

  virtual ~LteUeNetDevice (void);
  virtual void DoDispose ();


  // inherited from NetDevice
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  

  Ptr<LteUeMac> GetMac (void);

  Ptr<LteUeRrc> GetRrc ();

  Ptr<LteUePhy> GetPhy (void) const;

  /**
   * \brief Set the targer eNB where the UE is registered
   * \param enb
   */
  void SetTargetEnb (Ptr<LteEnbNetDevice> enb);
  /**
   * \brief Get the targer eNB where the UE is registered
   * \return the pointer to the enb
   */
  Ptr<LteEnbNetDevice> GetTargetEnb (void);

  uint64_t GetImsi ();


protected:
  // inherited from Object
  virtual void DoStart (void);


private:

  /**
   * Some attributes are exported as
   * attributes of the LteUeNetDevice from a user perspective,  but
   * are actually used also in other modules as well (the RRC, the
   * PHY...). This methods takes care of updating the
   * configuration of all modules so that their copy of the attribute
   * values is in sync with the one in the LteUeNetDevice.
   */
  void UpdateConfig (void);

  Ptr<LteEnbNetDevice> m_targetEnb;

  Ptr<LteUeMac> m_mac;
  Ptr<LteUePhy> m_phy;
  Ptr<LteUeRrc> m_rrc;

  uint64_t m_imsi;
  static uint64_t m_imsiCounter;

  
};

} // namespace ns3

#endif /* LTE_UE_NET_DEVICE_H */
