/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include "udp-echo-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpEchoServerApplication");
NS_OBJECT_ENSURE_REGISTERED (UdpEchoServer);

TypeId
UdpEchoServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpEchoServer")
    .SetParent<Application> ()
    .AddConstructor<UdpEchoServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&UdpEchoServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
	.AddAttribute ("PriorityGroup", "The priority group of this flow",
				   UintegerValue (0),
				   MakeUintegerAccessor (&UdpEchoServer::m_pg),
				   MakeUintegerChecker<uint16_t> ())
	.AddAttribute ("ChunkSize", 
				   "The chunk size can be sent before getting an ack",
				   UintegerValue (4000000),
				   MakeUintegerAccessor (&UdpEchoServer::m_chunk),
				   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

UdpEchoServer::UdpEchoServer ()
{
	m_received = 0;
  NS_LOG_FUNCTION_NOARGS ();
}

UdpEchoServer::~UdpEchoServer()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_socket6 = 0;
}

void
UdpEchoServer::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void 
UdpEchoServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      m_socket6->Bind (local6);
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpEchoServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&UdpEchoServer::HandleRead, this));
}

void 
UdpEchoServer::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
UdpEchoServer::HandleRead (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());

		  packet->RemoveAllPacketTags ();
		  packet->RemoveAllByteTags ();

		  m_received += packet->GetSize();
		  if (m_received>=m_chunk)
		  {
			  m_received = 0;
			  SeqTsHeader seqTs;
			  seqTs.SetSeq (1);
			  seqTs.SetPG (m_pg);
			  Ptr<Packet> p = Create<Packet> (0);
			  p->AddHeader (seqTs);
			  NS_LOG_LOGIC ("Echoing packet");
			  socket->SendTo (p, 0, from);	  //ack
		  } 
	  }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }


	  /*
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
		*/
    }
}

} // Namespace ns3
