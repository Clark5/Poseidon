/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "wifi-phy-state-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"

NS_LOG_COMPONENT_DEFINE ("WifiPhyStateHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WifiPhyStateHelper);

TypeId
WifiPhyStateHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhyStateHelper")
    .SetParent<Object> ()
    .AddConstructor<WifiPhyStateHelper> ()
    .AddTraceSource ("State",
                     "The state of the PHY layer",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_stateLogger))
    .AddTraceSource ("RxOk",
                     "A packet has been received successfully.",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_rxOkTrace))
    .AddTraceSource ("RxError",
                     "A packet has been received unsuccessfully.",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_rxErrorTrace))
    .AddTraceSource ("Tx", "Packet transmission is starting.",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_txTrace))
  ;
  return tid;
}

WifiPhyStateHelper::WifiPhyStateHelper ()
  : m_rxing (false),
    m_endTx (Seconds (0)),
    m_endRx (Seconds (0)),
    m_endCcaBusy (Seconds (0)),
    m_endSwitching (Seconds (0)),
    m_startTx (Seconds (0)),
    m_startRx (Seconds (0)),
    m_startCcaBusy (Seconds (0)),
    m_startSwitching (Seconds (0)),
    m_previousStateChangeTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
}

void
WifiPhyStateHelper::SetReceiveOkCallback (WifiPhy::RxOkCallback callback)
{
  m_rxOkCallback = callback;
}
void
WifiPhyStateHelper::SetReceiveErrorCallback (WifiPhy::RxErrorCallback callback)
{
  m_rxErrorCallback = callback;
}
void
WifiPhyStateHelper::RegisterListener (WifiPhyListener *listener)
{
  m_listeners.push_back (listener);
}

bool
WifiPhyStateHelper::IsStateIdle (void)
{
  return (GetState () == WifiPhy::IDLE);
}
bool
WifiPhyStateHelper::IsStateBusy (void)
{
  return (GetState () != WifiPhy::IDLE);
}
bool
WifiPhyStateHelper::IsStateCcaBusy (void)
{
  return (GetState () == WifiPhy::CCA_BUSY);
}
bool
WifiPhyStateHelper::IsStateRx (void)
{
  return (GetState () == WifiPhy::RX);
}
bool
WifiPhyStateHelper::IsStateTx (void)
{
  return (GetState () == WifiPhy::TX);
}
bool
WifiPhyStateHelper::IsStateSwitching (void)
{
  return (GetState () == WifiPhy::SWITCHING);
}



Time
WifiPhyStateHelper::GetStateDuration (void)
{
  return Simulator::Now () - m_previousStateChangeTime;
}

Time
WifiPhyStateHelper::GetDelayUntilIdle (void)
{
  Time retval;

  switch (GetState ())
    {
    case WifiPhy::RX:
      retval = m_endRx - Simulator::Now ();
      break;
    case WifiPhy::TX:
      retval = m_endTx - Simulator::Now ();
      break;
    case WifiPhy::CCA_BUSY:
      retval = m_endCcaBusy - Simulator::Now ();
      break;
    case WifiPhy::SWITCHING:
      retval = m_endSwitching - Simulator::Now ();
      break;
    case WifiPhy::IDLE:
      retval = Seconds (0);
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      retval = Seconds (0);
      break;
    }
  retval = Max (retval, Seconds (0));
  return retval;
}

Time
WifiPhyStateHelper::GetLastRxStartTime (void) const
{
  return m_startRx;
}

enum WifiPhy::State
WifiPhyStateHelper::GetState (void)
{
  if (m_endTx > Simulator::Now ())
    {
      return WifiPhy::TX;
    }
  else if (m_rxing)
    {
      return WifiPhy::RX;
    }
  else if (m_endSwitching > Simulator::Now ())
    {
      return WifiPhy::SWITCHING;
    }
  else if (m_endCcaBusy > Simulator::Now ())
    {
      return WifiPhy::CCA_BUSY;
    }
  else
    {
      return WifiPhy::IDLE;
    }
}


void
WifiPhyStateHelper::NotifyTxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (duration);
    }
}
void
WifiPhyStateHelper::NotifyRxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart (duration);
    }
}
void
WifiPhyStateHelper::NotifyRxEndOk (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndOk ();
    }
}
void
WifiPhyStateHelper::NotifyRxEndError (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndError ();
    }
}
void
WifiPhyStateHelper::NotifyMaybeCcaBusyStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyMaybeCcaBusyStart (duration);
    }
}
void
WifiPhyStateHelper::NotifySwitchingStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifySwitchingStart (duration);
    }
}

void
WifiPhyStateHelper::LogPreviousIdleAndCcaBusyStates (void)
{
  Time now = Simulator::Now ();
  Time idleStart = Max (m_endCcaBusy, m_endRx);
  idleStart = Max (idleStart, m_endTx);
  idleStart = Max (idleStart, m_endSwitching);
  NS_ASSERT (idleStart <= now);
  if (m_endCcaBusy > m_endRx
      && m_endCcaBusy > m_endSwitching
      && m_endCcaBusy > m_endTx)
    {
      Time ccaBusyStart = Max (m_endTx, m_endRx);
      ccaBusyStart = Max (ccaBusyStart, m_startCcaBusy);
      ccaBusyStart = Max (ccaBusyStart, m_endSwitching);
      m_stateLogger (ccaBusyStart, idleStart - ccaBusyStart, WifiPhy::CCA_BUSY);
    }
  m_stateLogger (idleStart, now - idleStart, WifiPhy::IDLE);
}

void
WifiPhyStateHelper::SwitchToTx (Time txDuration, Ptr<const Packet> packet, WifiMode txMode,
                                WifiPreamble preamble, uint8_t txPower)
{
  m_txTrace (packet, txMode, preamble, txPower);
  NotifyTxStart (txDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case WifiPhy::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_rxing = false;
      m_stateLogger (m_startRx, now - m_startRx, WifiPhy::RX);
      m_endRx = now;
      break;
    case WifiPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhy::CCA_BUSY);
      } break;
    case WifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case WifiPhy::SWITCHING:
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_stateLogger (now, txDuration, WifiPhy::TX);
  m_previousStateChangeTime = now;
  m_endTx = now + txDuration;
  m_startTx = now;
}
void
WifiPhyStateHelper::SwitchToRx (Time rxDuration)
{
  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
  NS_ASSERT (!m_rxing);
  NotifyRxStart (rxDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case WifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case WifiPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhy::CCA_BUSY);
      } break;
    case WifiPhy::SWITCHING:
    case WifiPhy::RX:
    case WifiPhy::TX:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_previousStateChangeTime = now;
  m_rxing = true;
  m_startRx = now;
  m_endRx = now + rxDuration;
  NS_ASSERT (IsStateRx ());
}

void
WifiPhyStateHelper::SwitchToChannelSwitching (Time switchingDuration)
{
  NotifySwitchingStart (switchingDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case WifiPhy::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_rxing = false;
      m_stateLogger (m_startRx, now - m_startRx, WifiPhy::RX);
      m_endRx = now;
      break;
    case WifiPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhy::CCA_BUSY);
      } break;
    case WifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case WifiPhy::TX:
    case WifiPhy::SWITCHING:
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }

  if (now < m_endCcaBusy)
    {
      m_endCcaBusy = now;
    }

  m_stateLogger (now, switchingDuration, WifiPhy::SWITCHING);
  m_previousStateChangeTime = now;
  m_startSwitching = now;
  m_endSwitching = now + switchingDuration;
  NS_ASSERT (IsStateSwitching ());
}

void
WifiPhyStateHelper::SwitchFromRxEndOk (Ptr<Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble)
{
  m_rxOkTrace (packet, snr, mode, preamble);
  NotifyRxEndOk ();
  DoSwitchFromRx ();
  if (!m_rxOkCallback.IsNull ())
    {
      m_rxOkCallback (packet, snr, mode, preamble);
    }

}
void
WifiPhyStateHelper::SwitchFromRxEndError (Ptr<const Packet> packet, double snr)
{
  m_rxErrorTrace (packet, snr);
  NotifyRxEndError ();
  DoSwitchFromRx ();
  if (!m_rxErrorCallback.IsNull ())
    {
      m_rxErrorCallback (packet, snr);
    }
}

void
WifiPhyStateHelper::DoSwitchFromRx (void)
{
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_rxing);

  Time now = Simulator::Now ();
  m_stateLogger (m_startRx, now - m_startRx, WifiPhy::RX);
  m_previousStateChangeTime = now;
  m_rxing = false;

  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
}
void
WifiPhyStateHelper::SwitchMaybeToCcaBusy (Time duration)
{
  NotifyMaybeCcaBusyStart (duration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case WifiPhy::SWITCHING:
      break;
    case WifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case WifiPhy::CCA_BUSY:
      break;
    case WifiPhy::RX:
      break;
    case WifiPhy::TX:
      break;
    }
  m_startCcaBusy = now;
  m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
}

} // namespace ns3
