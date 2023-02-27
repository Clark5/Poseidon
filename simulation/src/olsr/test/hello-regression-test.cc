/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
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
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */

#include "hello-regression-test.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/pcap-file.h"
#include "ns3/olsr-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/abort.h"
#include "ns3/pcap-test.h"

namespace ns3
{
namespace olsr
{

const char * const HelloRegressionTest::PREFIX = "olsr-hello-regression-test";

HelloRegressionTest::HelloRegressionTest() : 
  TestCase ("Test OLSR Hello messages generation"),
  m_time (Seconds (5))
{
}

HelloRegressionTest::~HelloRegressionTest()
{
}

void
HelloRegressionTest::DoRun ()
{
  RngSeedManager::SetSeed (12345);
  RngSeedManager::SetRun (7);
  CreateNodes ();

  Simulator::Stop (m_time);
  Simulator::Run ();
  Simulator::Destroy ();

  CheckResults ();
}

void
HelloRegressionTest::CreateNodes ()
{
  // create 2 nodes
  NodeContainer c;
  c.Create (2);
  // install TCP/IP & OLSR
  OlsrHelper olsr;
  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr);
  internet.Install (c);
  // Assign OLSR RVs to specific streams
  int64_t streamsUsed = olsr.AssignStreams (c, 0);
  NS_TEST_ASSERT_MSG_EQ (streamsUsed, 2, "Should have assigned 2 streams");
  // create p2p channel & devices
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer nd = p2p.Install (c);
  // setup IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (nd);
  // setup PCAP traces
  p2p.EnablePcapAll (CreateTempDirFilename (PREFIX));
}

void
HelloRegressionTest::CheckResults ()
{
  for (uint32_t i = 0; i < 2; ++i)
    {
      NS_PCAP_TEST_EXPECT_EQ (PREFIX << "-" << i << "-1.pcap");
    }
}

}
}
