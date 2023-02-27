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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "arf-wifi-manager.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("ns3::ArfWifiManager");


namespace ns3 {

struct ArfWifiRemoteStation : public WifiRemoteStation
{
  uint32_t m_timer;
  uint32_t m_success;
  uint32_t m_failed;
  bool m_recovery;
  uint32_t m_retry;

  uint32_t m_timerTimeout;
  uint32_t m_successThreshold;

  uint32_t m_rate;
};

NS_OBJECT_ENSURE_REGISTERED (ArfWifiManager);

TypeId
ArfWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ArfWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .AddConstructor<ArfWifiManager> ()
    .AddAttribute ("TimerThreshold", "The 'timer' threshold in the ARF algorithm.",
                   UintegerValue (15),
                   MakeUintegerAccessor (&ArfWifiManager::m_timerThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SuccessThreshold",
                   "The minimum number of sucessfull transmissions to try a new rate.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&ArfWifiManager::m_successThreshold),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

ArfWifiManager::ArfWifiManager ()
{
}
ArfWifiManager::~ArfWifiManager ()
{
}
WifiRemoteStation *
ArfWifiManager::DoCreateStation (void) const
{
  ArfWifiRemoteStation *station = new ArfWifiRemoteStation ();

  station->m_successThreshold = m_successThreshold;
  station->m_timerTimeout = m_timerThreshold;
  station->m_rate = 0;
  station->m_success = 0;
  station->m_failed = 0;
  station->m_recovery = false;
  station->m_retry = 0;
  station->m_timer = 0;

  return station;
}

void
ArfWifiManager::DoReportRtsFailed (WifiRemoteStation *station)
{
}
/**
 * It is important to realize that "recovery" mode starts after failure of
 * the first transmission after a rate increase and ends at the first successful
 * transmission. Specifically, recovery mode transcends retransmissions boundaries.
 * Fundamentally, ARF handles each data transmission independently, whether it
 * is the initial transmission of a packet or the retransmission of a packet.
 * The fundamental reason for this is that there is a backoff between each data
 * transmission, be it an initial transmission or a retransmission.
 */
void
ArfWifiManager::DoReportDataFailed (WifiRemoteStation *st)
{
  ArfWifiRemoteStation *station = (ArfWifiRemoteStation *)st;
  station->m_timer++;
  station->m_failed++;
  station->m_retry++;
  station->m_success = 0;

  if (station->m_recovery)
    {
      NS_ASSERT (station->m_retry >= 1);
      if (station->m_retry == 1)
        {
          // need recovery fallback
          if (station->m_rate != 0)
            {
              station->m_rate--;
            }
        }
      station->m_timer = 0;
    }
  else
    {
      NS_ASSERT (station->m_retry >= 1);
      if (((station->m_retry - 1) % 2) == 1)
        {
          // need normal fallback
          if (station->m_rate != 0)
            {
              station->m_rate--;
            }
        }
      if (station->m_retry >= 2)
        {
          station->m_timer = 0;
        }
    }
}
void
ArfWifiManager::DoReportRxOk (WifiRemoteStation *station,
                              double rxSnr, WifiMode txMode)
{
}
void ArfWifiManager::DoReportRtsOk (WifiRemoteStation *station,
                                    double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_DEBUG ("station=" << station << " rts ok");
}
void ArfWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                     double ackSnr, WifiMode ackMode, double dataSnr)
{
  ArfWifiRemoteStation *station = (ArfWifiRemoteStation *) st;
  station->m_timer++;
  station->m_success++;
  station->m_failed = 0;
  station->m_recovery = false;
  station->m_retry = 0;
  NS_LOG_DEBUG ("station=" << station << " data ok success=" << station->m_success << ", timer=" << station->m_timer);
  if ((station->m_success == m_successThreshold
       || station->m_timer == m_timerThreshold)
      && (station->m_rate < (station->m_state->m_operationalRateSet.size () - 1)))
    {
      NS_LOG_DEBUG ("station=" << station << " inc rate");
      station->m_rate++;
      station->m_timer = 0;
      station->m_success = 0;
      station->m_recovery = true;
    }
}
void
ArfWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *station)
{
}
void
ArfWifiManager::DoReportFinalDataFailed (WifiRemoteStation *station)
{
}

WifiMode
ArfWifiManager::DoGetDataMode (WifiRemoteStation *st, uint32_t size)
{
  ArfWifiRemoteStation *station = (ArfWifiRemoteStation *) st;
  return GetSupported (station, station->m_rate);
}
WifiMode
ArfWifiManager::DoGetRtsMode (WifiRemoteStation *st)
{
  // XXX: we could/should implement the Arf algorithm for
  // RTS only by picking a single rate within the BasicRateSet.
  ArfWifiRemoteStation *station = (ArfWifiRemoteStation *) st;
  return GetSupported (station, 0);
}

bool
ArfWifiManager::IsLowLatency (void) const
{
  return true;
}

} // namespace ns3
