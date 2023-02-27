/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"


using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer nodes;

  // create an array of empty nodes for testing purposes 
  nodes.Create (120);

  MobilityHelper mobility;
  // setup the grid itself: objects are layed out
  // started from (-100,-100) with 20 objects per row, 
  // the x interval between each object is 5 meters
  // and the y interval between each object is 20 meters
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (-100.0),
                                 "MinY", DoubleValue (-100.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (20.0),
                                 "GridWidth", UintegerValue (20),
                                 "LayoutType", StringValue ("RowFirst"));
  // each object will be attached a static position.
  // i.e., once set by the "position allocator", the
  // position will never change.
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // finalize the setup by attaching to each object
  // in the input array a position and initializing
  // this position with the calculated coordinates.
  mobility.Install (nodes);

  // iterate our nodes and print their position.
  for (NodeContainer::Iterator j = nodes.Begin ();
       j != nodes.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Vector pos = position->GetPosition ();
      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }

  Simulator::Destroy ();
  return 0;
}
