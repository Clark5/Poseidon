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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 */

#include "lte-common.h"
#include <ns3/log.h>
#include <ns3/abort.h>

NS_LOG_COMPONENT_DEFINE ("LteCommon");

namespace ns3 {


LteFlowId_t::LteFlowId_t ()
{
}

LteFlowId_t::LteFlowId_t (const uint16_t a, const uint8_t b)
  : m_rnti (a),
    m_lcId (b)
{
}

bool
operator == (const LteFlowId_t &a, const LteFlowId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_lcId == b.m_lcId) );
}

bool
operator < (const LteFlowId_t& a, const LteFlowId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_lcId < b.m_lcId) ) );
}

ImsiLcidPair_t::ImsiLcidPair_t ()
{
}

ImsiLcidPair_t::ImsiLcidPair_t (const uint64_t a, const uint8_t b)
  : m_imsi (a),
    m_lcId (b)
{
}

bool
operator == (const ImsiLcidPair_t &a, const ImsiLcidPair_t &b)
{
  return ((a.m_imsi == b.m_imsi) && (a.m_lcId == b.m_lcId));
}

bool
operator < (const ImsiLcidPair_t& a, const ImsiLcidPair_t& b)
{
  return ((a.m_imsi < b.m_imsi) || ((a.m_imsi == b.m_imsi) && (a.m_lcId
                                                               < b.m_lcId)));
}


LteUeConfig_t::LteUeConfig_t ()
{
}



bool
operator == (const LteUeConfig_t &a, const LteUeConfig_t &b)
{
  return (a.m_rnti == b.m_rnti);
}

bool
operator < (const LteUeConfig_t& a, const LteUeConfig_t& b)
{
  return (a.m_rnti < b.m_rnti);
}


uint16_t
LteFfConverter::double2fpS11dot3 (double val)
{
  // convert from double to fixed point notation Sxxxxxxxxxxx.xxx
  // truncate val to notation limits
  if (val > 4095.88)
    {
      val = 4095.88;
    }
  if (val < -4096)
    {
      val = -4096;
    }
  int16_t valFp = (int16_t)(val * 8);
  return (valFp);
}

double
LteFfConverter::fpS11dot3toDouble (uint16_t val)
{
  // convert from fixed point notation Sxxxxxxxxxxx.xxx to double
  double valD = ((int16_t)val) / 8.0;
  return (valD);
}

double 
LteFfConverter::getMinFpS11dot3Value ()
{
  return (-4096);        // -4096 = 0x8000 = 1000 0000 0000 0000 b
}

//static double g_lowestFpS11dot3Value = -4096; // 0x8001 (1000 0000 0000 0000)


uint32_t BufferSizeLevelBsrTable[64] = {
  
  0, 10, 12, 14, 17, 19, 22, 26, 31, 36, 42, 49, 57, 67, 78, 91, 
  107, 125, 146, 171, 200, 234, 274, 321, 376, 440, 515, 603, 
  706, 826, 967, 1132, 1326, 1552, 1817, 2127, 2490, 2915, 3413,
  3995, 4677, 5476, 6411, 7505, 8787, 10287, 12043, 14099, 16507,
  19325, 22624, 26487, 31009, 36304, 42502, 49759, 58255,
  68201, 79846, 93749, 109439, 128125, 150000, 150000
  
};

uint32_t
BufferSizeLevelBsr::BsrId2BufferSize (uint8_t val)
{
  NS_ABORT_MSG_UNLESS (val < 64, "val = " << val << " is out of range");
  return BufferSizeLevelBsrTable[val];
}

uint8_t
BufferSizeLevelBsr::BufferSize2BsrId (uint32_t val)
{
  int index = 0;
  if (BufferSizeLevelBsrTable[63] < val)
    {
      index = 63;
    }
  else
    {
      while (BufferSizeLevelBsrTable[index] < val)
        {
          NS_ASSERT (index < 64);
          index++;
        }
    }
    
  return (index);
}


uint8_t
TransmissionModesLayers::TxMode2LayerNum (uint8_t txMode)
{
  uint8_t nLayer = 0;
  switch (txMode)
    {
    case 0: // Tx MODE 1: SISO
      nLayer = 1;
      break;
    case 1: // Tx MODE 2: MIMO Tx Diversity
      nLayer = 1;
      break;
    case 2: // Tx MODE 3: MIMO Spatial Multiplexity Open Loop
      nLayer = 2;
      break;
    case 3: // Tx MODE 4: MIMO Spatial Multiplexity Closed Loop
      nLayer = 2;
      break;
    case 4: // Tx MODE 5: MIMO Multi-User
      nLayer = 2;
      break;
    case 5: // Tx MODE 6: Closer loop single layer percoding
      nLayer = 1;
      break;
    case 6: // Tx MODE 7: Single antenna port 5
      nLayer = 1;
      break;
    }
  return (nLayer);
}


}; // namespace ns3

