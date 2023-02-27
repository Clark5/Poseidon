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
#include "wifi-mac.h"
#include "dcf.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WifiMac);

Time
WifiMac::GetDefaultMaxPropagationDelay (void)
{
  // 1000m
  return Seconds (1000.0 / 300000000.0);
}
Time
WifiMac::GetDefaultSlot (void)
{
  // 802.11-a specific
  return MicroSeconds (9);
}
Time
WifiMac::GetDefaultSifs (void)
{
  // 802.11-a specific
  return MicroSeconds (16);
}
Time
WifiMac::GetDefaultEifsNoDifs (void)
{
  return GetDefaultSifs () + GetDefaultCtsAckDelay ();
}
Time
WifiMac::GetDefaultCtsAckDelay (void)
{
  // 802.11-a specific: 6mb/s
  return MicroSeconds (44);
}
Time
WifiMac::GetDefaultCtsAckTimeout (void)
{
  /* Cts_Timeout and Ack_Timeout are specified in the Annex C
     (Formal description of MAC operation, see details on the
     Trsp timer setting at page 346)
  */
  Time ctsTimeout = GetDefaultSifs ();
  ctsTimeout += GetDefaultCtsAckDelay ();
  ctsTimeout += MicroSeconds (GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2);
  ctsTimeout += GetDefaultSlot ();
  return ctsTimeout;
}

Time
WifiMac::GetDefaultBasicBlockAckDelay (void)
{
  // This value must be rivisited
  return MicroSeconds (250);
}
Time
WifiMac::GetDefaultCompressedBlockAckDelay (void)
{
  // This value must be rivisited
  return MicroSeconds (68);
}
Time
WifiMac::GetDefaultBasicBlockAckTimeout (void)
{
  Time blockAckTimeout = GetDefaultSifs ();
  blockAckTimeout += GetDefaultBasicBlockAckDelay ();
  blockAckTimeout += MicroSeconds (GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2);
  blockAckTimeout += GetDefaultSlot ();
  return blockAckTimeout;
}
Time
WifiMac::GetDefaultCompressedBlockAckTimeout (void)
{
  Time blockAckTimeout = GetDefaultSifs ();
  blockAckTimeout += GetDefaultCompressedBlockAckDelay ();
  blockAckTimeout += MicroSeconds (GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2);
  blockAckTimeout += GetDefaultSlot ();
  return blockAckTimeout;
}

void
WifiMac::SetBasicBlockAckTimeout (Time blockAckTimeout)
{
  //this method must be implemented by QoS WifiMacs
}

Time
WifiMac::GetBasicBlockAckTimeout (void) const
{
  //this method must be implemented by QoS WifiMacs
  return MicroSeconds (0);
}

void
WifiMac::SetCompressedBlockAckTimeout (Time blockAckTimeout)
{
  //this methos must be implemented by QoS WifiMacs
}

Time
WifiMac::GetCompressedBlockAckTimeout (void) const
{
  //this method must be implemented by QoS WifiMacs
  return MicroSeconds (0);
}

TypeId
WifiMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiMac")
    .SetParent<Object> ()
    .AddAttribute ("CtsTimeout", "When this timeout expires, the RTS/CTS handshake has failed.",
                   TimeValue (GetDefaultCtsAckTimeout ()),
                   MakeTimeAccessor (&WifiMac::SetCtsTimeout,
                                     &WifiMac::GetCtsTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("AckTimeout", "When this timeout expires, the DATA/ACK handshake has failed.",
                   TimeValue (GetDefaultCtsAckTimeout ()),
                   MakeTimeAccessor (&WifiMac::GetAckTimeout,
                                     &WifiMac::SetAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("BasicBlockAckTimeout", "When this timeout expires, the BASIC_BLOCK_ACK_REQ/BASIC_BLOCK_ACK handshake has failed.",
                   TimeValue (GetDefaultBasicBlockAckTimeout ()),
                   MakeTimeAccessor (&WifiMac::GetBasicBlockAckTimeout,
                                     &WifiMac::SetBasicBlockAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("CompressedBlockAckTimeout", "When this timeout expires, the COMPRESSED_BLOCK_ACK_REQ/COMPRESSED_BLOCK_ACK handshake has failed.",
                   TimeValue (GetDefaultCompressedBlockAckTimeout ()),
                   MakeTimeAccessor (&WifiMac::GetCompressedBlockAckTimeout,
                                     &WifiMac::SetCompressedBlockAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("Sifs", "The value of the SIFS constant.",
                   TimeValue (GetDefaultSifs ()),
                   MakeTimeAccessor (&WifiMac::SetSifs,
                                     &WifiMac::GetSifs),
                   MakeTimeChecker ())
    .AddAttribute ("EifsNoDifs", "The value of EIFS-DIFS",
                   TimeValue (GetDefaultEifsNoDifs ()),
                   MakeTimeAccessor (&WifiMac::SetEifsNoDifs,
                                     &WifiMac::GetEifsNoDifs),
                   MakeTimeChecker ())
    .AddAttribute ("Slot", "The duration of a Slot.",
                   TimeValue (GetDefaultSlot ()),
                   MakeTimeAccessor (&WifiMac::SetSlot,
                                     &WifiMac::GetSlot),
                   MakeTimeChecker ())
    .AddAttribute ("Pifs", "The value of the PIFS constant.",
                   TimeValue (GetDefaultSifs () + GetDefaultSlot ()),
                   MakeTimeAccessor (&WifiMac::SetPifs,
                                     &WifiMac::GetPifs),
                   MakeTimeChecker ())
    .AddAttribute ("MaxPropagationDelay", "The maximum propagation delay. Unused for now.",
                   TimeValue (GetDefaultMaxPropagationDelay ()),
                   MakeTimeAccessor (&WifiMac::m_maxPropagationDelay),
                   MakeTimeChecker ())
    .AddAttribute ("Ssid", "The ssid we want to belong to.",
                   SsidValue (Ssid ("default")),
                   MakeSsidAccessor (&WifiMac::GetSsid,
                                     &WifiMac::SetSsid),
                   MakeSsidChecker ())
    .AddTraceSource ("MacTx",
                     "A packet has been received from higher layers and is being processed in preparation for "
                     "queueing for transmission.",
                     MakeTraceSourceAccessor (&WifiMac::m_macTxTrace))
    .AddTraceSource ("MacTxDrop",
                     "A packet has been dropped in the MAC layer before being queued for transmission.",
                     MakeTraceSourceAccessor (&WifiMac::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&WifiMac::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&WifiMac::m_macRxTrace))
    .AddTraceSource ("MacRxDrop",
                     "A packet has been dropped in the MAC layer after it has been passed up from the physical "
                     "layer.",
                     MakeTraceSourceAccessor (&WifiMac::m_macRxDropTrace))
#if 0
    // Not currently implemented in this device
    .AddTraceSource ("Sniffer",
                     "Trace source simulating a non-promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&WifiMac::m_snifferTrace))
#endif
  ;

  return tid;
}

void
WifiMac::SetMaxPropagationDelay (Time delay)
{
  m_maxPropagationDelay = delay;
}

Time
WifiMac::GetMsduLifetime (void) const
{
  return Seconds (10);
}
Time
WifiMac::GetMaxPropagationDelay (void) const
{
  return m_maxPropagationDelay;
}

void
WifiMac::NotifyTx (Ptr<const Packet> packet)
{
  m_macTxTrace (packet);
}

void
WifiMac::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_macTxDropTrace (packet);
}

void
WifiMac::NotifyRx (Ptr<const Packet> packet)
{
  m_macRxTrace (packet);
}

void
WifiMac::NotifyPromiscRx (Ptr<const Packet> packet)
{
  m_macPromiscRxTrace (packet);
}

void
WifiMac::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_macRxDropTrace (packet);
}

void
WifiMac::ConfigureStandard (enum WifiPhyStandard standard)
{
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      Configure80211a ();
      break;
    case WIFI_PHY_STANDARD_80211b:
      Configure80211b ();
      break;
    case WIFI_PHY_STANDARD_80211g:
      Configure80211g ();
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      Configure80211_10Mhz ();
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      Configure80211_5Mhz ();
      break;
    case WIFI_PHY_STANDARD_holland:
      Configure80211a ();
      break;
    case WIFI_PHY_STANDARD_80211p_CCH:
      Configure80211p_CCH ();
      break;
    case WIFI_PHY_STANDARD_80211p_SCH:
      Configure80211p_SCH ();
      break;
    default:
      NS_ASSERT (false);
      break;
    }
  FinishConfigureStandard (standard);
}

void
WifiMac::Configure80211a (void)
{
  SetSifs (MicroSeconds (16));
  SetSlot (MicroSeconds (9));
  SetEifsNoDifs (MicroSeconds (16 + 44));
  SetPifs (MicroSeconds (16 + 9));
  SetCtsTimeout (MicroSeconds (16 + 44 + 9 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  SetAckTimeout (MicroSeconds (16 + 44 + 9 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
}

void
WifiMac::Configure80211b (void)
{
  SetSifs (MicroSeconds (10));
  SetSlot (MicroSeconds (20));
  SetEifsNoDifs (MicroSeconds (10 + 304));
  SetPifs (MicroSeconds (10 + 20));
  SetCtsTimeout (MicroSeconds (10 + 304 + 20 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  SetAckTimeout (MicroSeconds (10 + 304 + 20 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
}

void
WifiMac::Configure80211g (void)
{
  SetSifs (MicroSeconds (10));
  // Note no support for Short Slot Time as yet
  SetSlot (MicroSeconds (20));
  SetEifsNoDifs (MicroSeconds (10 + 304));
  SetPifs (MicroSeconds (10 + 20));
  SetCtsTimeout (MicroSeconds (10 + 304 + 20 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  SetAckTimeout (MicroSeconds (10 + 304 + 20 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
}

void
WifiMac::Configure80211_10Mhz (void)
{
  SetSifs (MicroSeconds (32));
  SetSlot (MicroSeconds (13));
  SetEifsNoDifs (MicroSeconds (32 + 88));
  SetPifs (MicroSeconds (32 + 13));
  SetCtsTimeout (MicroSeconds (32 + 88 + 13 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  SetAckTimeout (MicroSeconds (32 + 88 + 13 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
}

void
WifiMac::Configure80211_5Mhz (void)
{
  SetSifs (MicroSeconds (64));
  SetSlot (MicroSeconds (21));
  SetEifsNoDifs (MicroSeconds (64 + 176));
  SetPifs (MicroSeconds (64 + 21));
  SetCtsTimeout (MicroSeconds (64 + 176 + 21 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  SetAckTimeout (MicroSeconds (64 + 176 + 21 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
}

void
WifiMac::Configure80211p_CCH (void)
{
  Configure80211_10Mhz ();
}

void
WifiMac::Configure80211p_SCH (void)
{
  Configure80211_10Mhz ();
}

void
WifiMac::ConfigureDcf (Ptr<Dcf> dcf, uint32_t cwmin, uint32_t cwmax, enum AcIndex ac)
{
  /* see IEE802.11 section 7.3.2.29 */
  switch (ac)
    {
    case AC_VO:
      dcf->SetMinCw ((cwmin + 1) / 4 - 1);
      dcf->SetMaxCw ((cwmin + 1) / 2 - 1);
      dcf->SetAifsn (2);
      break;
    case AC_VI:
      dcf->SetMinCw ((cwmin + 1) / 2 - 1);
      dcf->SetMaxCw (cwmin);
      dcf->SetAifsn (2);
      break;
    case AC_BE:
      dcf->SetMinCw (cwmin);
      dcf->SetMaxCw (cwmax);
      dcf->SetAifsn (3);
      break;
    case AC_BK:
      dcf->SetMinCw (cwmin);
      dcf->SetMaxCw (cwmax);
      dcf->SetAifsn (7);
      break;
    case AC_BE_NQOS:
      dcf->SetMinCw (cwmin);
      dcf->SetMaxCw (cwmax);
      dcf->SetAifsn (2);
      break;
    case AC_UNDEF:
      NS_FATAL_ERROR ("I don't know what to do with this");
      break;
    }
}

void
WifiMac::ConfigureCCHDcf (Ptr<Dcf> dcf, uint32_t cwmin, uint32_t cwmax, enum AcIndex ac)
{
  /* see IEEE 1609.4-2006 section 6.3.1, Table 1 */
  switch (ac)
    {
    case AC_VO:
      dcf->SetMinCw ((cwmin + 1) / 4 - 1);
      dcf->SetMaxCw ((cwmin + 1) / 2 - 1);
      dcf->SetAifsn (2);
      break;
    case AC_VI:
      dcf->SetMinCw ((cwmin + 1) / 4 - 1);
      dcf->SetMaxCw ((cwmin + 1) / 2 - 1);
      dcf->SetAifsn (3);
      break;
    case AC_BE:
      dcf->SetMinCw ((cwmin + 1) / 2 - 1);
      dcf->SetMaxCw (cwmin);
      dcf->SetAifsn (6);
      break;
    case AC_BK:
      dcf->SetMinCw (cwmin);
      dcf->SetMaxCw (cwmax);
      dcf->SetAifsn (9);
      break;
    case AC_BE_NQOS:
      dcf->SetMinCw (cwmin);
      dcf->SetMaxCw (cwmax);
      dcf->SetAifsn (2);
      break;
    case AC_UNDEF:
      NS_FATAL_ERROR ("I don't know what to do with this");
      break;
    }
}
} // namespace ns3
