/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#ifndef LTE_PDCP_H
#define LTE_PDCP_H

#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include "ns3/object.h"

#include "ns3/lte-pdcp-sap.h"
#include "ns3/lte-rlc-sap.h"

namespace ns3 {

/**
 * LTE PDCP entity, see 3GPP TS 36.323
 */
class LtePdcp : public Object // SimpleRefCount<LtePdcp>
{
  friend class LtePdcpSpecificLteRlcSapUser;
  friend class LtePdcpSpecificLtePdcpSapProvider<LtePdcp>;
public:
  LtePdcp ();
  virtual ~LtePdcp ();
  static TypeId GetTypeId (void);

  void Start ();

  /**
   *
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);

  /**
   *
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   *
   *
   * \param s the PDCP SAP user to be used by this LTE_PDCP
   */
  void SetLtePdcpSapUser (LtePdcpSapUser * s);

  /**
   *
   *
   * \param s the PDCP SAP Provider interface offered to the RRC by this LTE_PDCP
   */
  LtePdcpSapProvider* GetLtePdcpSapProvider ();

  /**
   *
   *
   * \param s the RLC SAP Provider to be used by this LTE_PDCP
   */
  void SetLteRlcSapProvider (LteRlcSapProvider * s);

  /**
   *
   *
   * \param s the RLC SAP User interface offered to the RLC by this LTE_PDCP
   */
  LteRlcSapUser* GetLteRlcSapUser ();


protected:
  // Interface provided to upper RRC entity
  virtual void DoTransmitRrcPdu (Ptr<Packet> p);

  LtePdcpSapUser* m_pdcpSapUser;
  LtePdcpSapProvider* m_pdcpSapProvider;

  // Interface provided to lower RLC entity
  virtual void DoReceivePdu (Ptr<Packet> p);

  LteRlcSapUser* m_rlcSapUser;
  LteRlcSapProvider* m_rlcSapProvider;

  uint16_t m_rnti;
  uint8_t m_lcid;

  /**
   * Used to inform of a PDU delivery to the RLC SAP provider.
   * The parameters are RNTI, LCID and bytes delivered
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception from the RLC SAP user.
   * The parameters are RNTI, LCID, bytes delivered and delivery delay in nanoseconds. 
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

private:
  /**
   * State variables. See section 7.1 in TS 36.323
   */
  uint16_t m_txSequenceNumber;
  uint16_t m_rxSequenceNumber;

  /**
   * Constants. See section 7.2 in TS 36.323
   */
  static const uint16_t m_maxPdcpSn = 4095;

};


} // namespace ns3

#endif // LTE_PDCP_H
