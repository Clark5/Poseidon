/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/error-model.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/lte-rlc-header.h"
#include "ns3/lte-rlc-um.h"
#include "ns3/config-store.h"

#include "lte-test-rlc-am-e2e.h"
#include "lte-simple-helper.h"
#include "lte-test-entities.h"


NS_LOG_COMPONENT_DEFINE ("LteRlcAmE2eTest");

namespace ns3 {


/**
 * Test x.x.x RLC AM: End-to-end flow
 */

/**
 * TestSuite
 */

LteRlcAmE2eTestSuite::LteRlcAmE2eTestSuite ()
  : TestSuite ("lte-rlc-am-e2e", SYSTEM)
{
  // NS_LOG_INFO ("Creating LteRlcAmE2eTestSuite");

  double losses[] = {0.0, 0.10, 0.25, 0.50, 0.75, 0.90, 0.95};
  uint32_t seeds[] = {1111, 2222, 3333, 4444, 5555, 6666, 7777, 8888, 9999, 10101};

  for ( uint32_t l = 0 ; l < (sizeof (losses) / sizeof (double)) ; l++ )
    {
      for ( uint32_t s = 0 ; s < (sizeof (seeds) / sizeof (uint32_t)) ; s++ )
        {
          std::ostringstream name;
          name << " Losses = " << losses[l] << "%. Seed = " << seeds[s];
          AddTestCase (new LteRlcAmE2eTestCase (name.str (), seeds[s], losses[l]));
        }
    }
}

static LteRlcAmE2eTestSuite lteRlcAmE2eTestSuite;


/**
 * TestCase
 */

LteRlcAmE2eTestCase::LteRlcAmE2eTestCase (std::string name, uint32_t seed, double losses)
  : TestCase (name)
{
  // NS_LOG_UNCOND ("Creating LteRlcAmTestingTestCase: " + name);

  m_seed = seed;
  m_losses = losses;

  m_dlDrops = 0;
  m_ulDrops = 0;
}

LteRlcAmE2eTestCase::~LteRlcAmE2eTestCase ()
{
}


void
LteRlcAmE2eTestCase::DlDropEvent (Ptr<const Packet> p)
{
  // NS_LOG_FUNCTION (this);
  m_dlDrops++;
}

void
LteRlcAmE2eTestCase::UlDropEvent (Ptr<const Packet> p)
{
  // NS_LOG_FUNCTION (this);
  m_ulDrops++;
}

void
LteRlcAmE2eTestCase::DoRun (void)
{
  uint16_t numberOfNodes = 1;

  // LogLevel level = (LogLevel) (LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_PREFIX_FUNC);
  // LogComponentEnable ("LteRlcAmE2eTest", level);
  // LogComponentEnable ("ErrorModel", level);
  // LogComponentEnable ("LteSimpleHelper", level);
  // LogComponentEnable ("LteSimpleNetDevice", level);
  // LogComponentEnable ("SimpleNetDevice", level);
  // LogComponentEnable ("SimpleChannel", level);
  // LogComponentEnable ("LteTestEntities", level);
  // LogComponentEnable ("LtePdcp", level);
  // LogComponentEnable ("LteRlc", level);
  // LogComponentEnable ("LteRlcUm", level);
  // LogComponentEnable ("LteRlcAm", level);

  RngSeedManager::SetSeed (m_seed);

  Ptr<LteSimpleHelper> lteSimpleHelper = CreateObject<LteSimpleHelper> ();
  // lteSimpleHelper->EnableLogComponents ();
  // lteSimpleHelper->EnableTraces ();

  lteSimpleHelper->SetAttribute ("RlcEntity", StringValue ("RlcAm"));

  // eNB and UE nodes
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numberOfNodes);
  ueNodes.Create (numberOfNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteSimpleHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteSimpleHelper->InstallUeDevice (ueNodes);

  // Note: Just one eNB and UE is supported. Everything is done in InstallEnbDevice and InstallUeDevice

  // Attach one UE per eNodeB
  // for (uint16_t i = 0; i < numberOfNodes; i++)
  //   {
  //     lteSimpleHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
  //   }

  //   lteSimpleHelper->ActivateEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());


  // Error models: downlink and uplink
  Ptr<RateErrorModel> dlEm = CreateObject<RateErrorModel> ();
  dlEm->SetAttribute ("ErrorRate", DoubleValue (m_losses));
  dlEm->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

//   Ptr<RateErrorModel> ueEm = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"));
//   ueEm->SetAttribute ("ErrorRate", DoubleValue (m_losses));
//   ueEm->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

  // The below hooks will cause drops and receptions to be counted
  ueLteDevs.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (dlEm));
  ueLteDevs.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&LteRlcAmE2eTestCase::DlDropEvent, this));
//   enbLteDevs.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (enbEm));
//   enbLteDevs.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&LteRlcAmE2eTestCase::EnbDropEvent, this));

  // Sending packets from RRC layer
  lteSimpleHelper->m_enbRrc->SetArrivalTime (Seconds (0.010));
  lteSimpleHelper->m_enbRrc->SetPduSize (100);

  // MAC sends transmission opportunities (TxOpp)
  lteSimpleHelper->m_enbMac->SetTxOppSize (150);
  lteSimpleHelper->m_enbMac->SetTxOppTime (Seconds (0.005));
  lteSimpleHelper->m_enbMac->SetTxOpportunityMode (LteTestMac::RANDOM_MODE);

  // MAC sends transmission opportunities (TxOpp)
  lteSimpleHelper->m_ueMac->SetTxOppSize (140);
  lteSimpleHelper->m_ueMac->SetTxOppTime (Seconds (0.015));
  lteSimpleHelper->m_ueMac->SetTxOpportunityMode (LteTestMac::RANDOM_MODE);

  // Start/Stop pseudo-application at RRC layer
  Simulator::Schedule (Seconds (0.100), &LteTestRrc::Start, lteSimpleHelper->m_enbRrc);
  Simulator::Schedule (Seconds (10.100), &LteTestRrc::Stop, lteSimpleHelper->m_enbRrc);


  Simulator::Stop (Seconds (100.000));
  Simulator::Run ();

  uint32_t txEnbRrcPdus = lteSimpleHelper->m_enbRrc->GetTxPdus ();
  uint32_t rxUeRrcPdus = lteSimpleHelper->m_ueRrc->GetRxPdus ();

  // NS_LOG_INFO ("Seed = " << m_seed);
  // NS_LOG_INFO ("Losses (%) = " << uint32_t (m_losses * 100));

  // NS_LOG_INFO ("dl dev drops = " << m_dlDrops);
  // NS_LOG_INFO ("ul dev drops = " << m_ulDrops);

  // NS_LOG_INFO ("eNB tx RRC count = " << txEnbRrcPdus);
  // NS_LOG_INFO ("eNB rx RRC count = " << rxEnbRrcPdus);
  // NS_LOG_INFO ("UE tx RRC count = " << txUeRrcPdus);
  // NS_LOG_INFO ("UE rx RRC count = " << rxUeRrcPdus);

  NS_LOG_INFO (m_seed << "\t" << m_losses << "\t" << txEnbRrcPdus << "\t" << rxUeRrcPdus << "\t" << m_dlDrops);

  NS_TEST_ASSERT_MSG_EQ (txEnbRrcPdus, rxUeRrcPdus,
                         "TX PDUs (" << txEnbRrcPdus << ") != RX PDUs (" << rxUeRrcPdus << ")");

  Simulator::Destroy ();
}

} // namespace ns3

