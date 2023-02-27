/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2004,2005,2006 INRIA
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
 * Author: Federico Maguolo <maguolof@dei.unipd.it>
 */

#include "cara-wifi-manager.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("Cara");


namespace ns3 {

struct CaraWifiRemoteStation : public WifiRemoteStation
{
  uint32_t m_timer;
  uint32_t m_success;
  uint32_t m_failed;
  uint32_t m_rate;
};

NS_OBJECT_ENSURE_REGISTERED (CaraWifiManager);

TypeId
CaraWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CaraWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .AddConstructor<CaraWifiManager> ()
    .AddAttribute ("ProbeThreshold",
                   "The number of consecutive transmissions failure to activate the RTS probe.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&CaraWifiManager::m_probeThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("FailureThreshold",
                   "The number of consecutive transmissions failure to decrease the rate.",
                   UintegerValue (2),
                   MakeUintegerAccessor (&CaraWifiManager::m_failureThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SuccessThreshold",
                   "The minimum number of sucessfull transmissions to try a new rate.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&CaraWifiManager::m_successThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Timeout",
                   "The 'timer' in the CARA algorithm",
                   UintegerValue (15),
                   MakeUintegerAccessor (&CaraWifiManager::m_timerTimeout),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

CaraWifiManager::CaraWifiManager ()
  : WifiRemoteStationManager ()
{
}
CaraWifiManager::~CaraWifiManager ()
{
}

WifiRemoteStation *
CaraWifiManager::DoCreateStation (void) const
{
  CaraWifiRemoteStation *station = new CaraWifiRemoteStation ();
  station->m_rate = 0;
  station->m_success = 0;
  station->m_failed = 0;
  station->m_timer = 0;
  return station;
}

void
CaraWifiManager::DoReportRtsFailed (WifiRemoteStation *st)
{
}

void
CaraWifiManager::DoReportDataFailed (WifiRemoteStation *st)
{
  CaraWifiRemoteStation *station = (CaraWifiRemoteStation *) st;
  NS_LOG_FUNCTION (station);
  station->m_timer++;
  station->m_failed++;
  station->m_success = 0;
  if (station->m_failed >= m_failureThreshold)
    {
      NS_LOG_DEBUG ("self=" << station << " dec rate");
      if (station->m_rate != 0)
        {
          station->m_rate--;
        }
      station->m_failed = 0;
      station->m_timer = 0;
    }
}
void
CaraWifiManager::DoReportRxOk (WifiRemoteStation *st,
                               double rxSnr, WifiMode txMode)
{
}
void
CaraWifiManager::DoReportRtsOk (WifiRemoteStation *st,
                                double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_DEBUG ("self=" << st << " rts ok");
}
void
CaraWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                 double ackSnr, WifiMode ackMode, double dataSnr)
{
  CaraWifiRemoteStation *station = (CaraWifiRemoteStation *) st;
  station->m_timer++;
  station->m_success++;
  station->m_failed = 0;
  NS_LOG_DEBUG ("self=" << station << " data ok success=" << station->m_success << ", timer=" << station->m_timer);
  if ((station->m_success == m_successThreshold
       || station->m_timer >= m_timerTimeout))
    {
      if (station->m_rate < GetNSupported (station) - 1)
        {
          station->m_rate++;
        }
      NS_LOG_DEBUG ("self=" << station << " inc rate=" << station->m_rate);
      station->m_timer = 0;
      station->m_success = 0;
    }
}
void
CaraWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *st)
{
}
void
CaraWifiManager::DoReportFinalDataFailed (WifiRemoteStation *st)
{
}

WifiMode
CaraWifiManager::DoGetDataMode (WifiRemoteStation *st,
                                uint32_t size)
{
  CaraWifiRemoteStation *station = (CaraWifiRemoteStation *) st;
  return GetSupported (station, station->m_rate);
}
WifiMode
CaraWifiManager::DoGetRtsMode (WifiRemoteStation *st)
{
  // XXX: we could/should implement the Arf algorithm for
  // RTS only by picking a single rate within the BasicRateSet.
  return GetSupported (st, 0);
}

bool
CaraWifiManager::DoNeedRts (WifiRemoteStation *st,
                            Ptr<const Packet> packet, bool normally)
{
  CaraWifiRemoteStation *station = (CaraWifiRemoteStation *) st;
  return normally || station->m_failed >= m_probeThreshold;
}

bool
CaraWifiManager::IsLowLatency (void) const
{
  return true;
}

} // namespace ns3
