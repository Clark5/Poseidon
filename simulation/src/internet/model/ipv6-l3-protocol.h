/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007-2009 Strasbourg University
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
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

#ifndef IPV6_L3_PROTOCOL_H
#define IPV6_L3_PROTOCOL_H

#include <list>

#include "ns3/traced-callback.h"
#include "ns3/net-device.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-header.h"

namespace ns3
{

class Node;
class Ipv6Interface;
class IpL4Protocol;
class Ipv6Route;
class Ipv6MulticastRoute;
class Ipv6RawSocketImpl;
class Icmpv6L4Protocol;
class Ipv6AutoconfiguredPrefix;

/**
 * \class Ipv6L3Protocol
 * \brief IPv6 layer implementation.
 *
 * This class contains two distinct groups of trace sources.  The
 * trace sources 'Rx' and 'Tx' are called, respectively, immediately
 * after receiving from the NetDevice and immediately before sending
 * to a NetDevice for transmitting a packet.  These are low level
 * trace sources that include the Ipv6Header already serialized into
 * the packet.  In contrast, the Drop, SendOutgoing, UnicastForward,
 * and LocalDeliver trace sources are slightly higher-level and pass
 * around the Ipv6Header as an explicit parameter and not as part of
 * the packet.
 */
class Ipv6L3Protocol : public Ipv6
{
public:
  /**
   * \brief Get the type ID of this class.
   * \return type ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief The protocol number for IPv6 (0x86DD).
   */
  static const uint16_t PROT_NUMBER;

  /**
   * \enum DropReason
   * \brief Reason why a packet has been dropped.
   */
  enum DropReason
  {
    DROP_TTL_EXPIRED = 1, /**< Packet TTL has expired */
    DROP_NO_ROUTE, /**< No route to host */
    DROP_INTERFACE_DOWN, /**< Interface is down so can not send packet */
    DROP_ROUTE_ERROR, /**< Route error */
    DROP_UNKNOWN_PROTOCOL, /**< Unknown L4 protocol */
  };

  /**
   * \brief Constructor.
   */
  Ipv6L3Protocol ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6L3Protocol ();

  /**
   * \brief Set node for this stack.
   * \param node node to set
   */
  void SetNode (Ptr<Node> node);

  /**
   * \brief Add an L4 protocol.
   * \param protocol L4 protocol
   */
  void Insert (Ptr<IpL4Protocol> protocol);

  /**
   * \brief Remove an L4 protocol.
   * \param protocol L4 protocol to remove
   */
  void Remove (Ptr<IpL4Protocol> protocol);

  /**
   * \brief Get L4 protocol by protocol number.
   * \param protocolNumber protocol number
   * \return corresponding Ipv6L4Protocol or 0 if not found
   */
  Ptr<IpL4Protocol> GetProtocol (int protocolNumber) const;

  /**
   * \brief Create raw IPv6 socket.
   * \return newly raw socket
   */
  Ptr<Socket> CreateRawSocket ();

  /**
   * \brief Remove raw IPv6 socket.
   * \param socket socket to remove
   */
  void DeleteRawSocket (Ptr<Socket> socket);

  /**
   * \brief Set the default TTL.
   * \param ttl TTL to set
   */
  void SetDefaultTtl (uint8_t ttl);

  /**
   * \brief Set the default TCLASS.
   * \param tclass TCLASS to set
   */
  void SetDefaultTclass (uint8_t tclass);

  /**
   * \brief Receive method when a packet arrive in the stack.
   * This method removes IPv6 header and forward up to L4 protocol.
   *
   * \param device network device
   * \param p the packet
   * \param protocol next header value
   * \param from address of the correspondant
   * \param to address of the destination
   * \param packetType type of the packet
   */
  void Receive (Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

  /**
   * \brief Higher-level layers call this method to send a packet
   * down the stack to the MAC and PHY layers.
   *
   * \param packet packet to send
   * \param source source address of packet
   * \param destination address of packet
   * \param Traffic Class of packet
   * \param protocol number of packet
   * \param route route to take
   */
  void Send (Ptr<Packet> packet, Ipv6Address source, Ipv6Address destination, uint8_t tClass, uint8_t protocol, Ptr<Ipv6Route> route);

  /**
   * \brief Set routing protocol for this stack.
   * \param routingProtocol IPv6 routing protocol to set
   */
  void SetRoutingProtocol (Ptr<Ipv6RoutingProtocol> routingProtocol);

  /**
   * \brief Get current routing protocol used.
   * \return routing protocol
   */
  Ptr<Ipv6RoutingProtocol> GetRoutingProtocol () const;

  /**
   * \brief Add IPv6 interface for a device.
   * \param device net device
   * \return interface index
   */
  uint32_t AddInterface (Ptr<NetDevice> device);

  /**
   * \brief Get an interface.
   * \param i interface index
   * \return IPv6 interface pointer
   */
  Ptr<Ipv6Interface> GetInterface (uint32_t i) const;

  /**
   * \brief Get current number of interface on this stack.
   * \return number of interface registered
   */
  uint32_t GetNInterfaces () const;

  /**
   * \brief Get interface index which has specified IPv6 address
   * \param addr IPv6 address
   * \return interface index or -1 if not found
   */
  int32_t GetInterfaceForAddress (Ipv6Address addr) const;

  /**
   * \brief Get interface index which match specified address/prefix.
   * \param addr IPv6 address
   * \param mask IPv6 prefix (mask)
   * \return interface index or -1 if not found
   */
  int32_t GetInterfaceForPrefix (Ipv6Address addr, Ipv6Prefix mask) const;

  /**
   * \brief Get interface index which is on a specified net device.
   * \param device net device
   */
  int32_t GetInterfaceForDevice (Ptr<const NetDevice> device) const;

  /**
   * \brief Add an address on interface.
   * \param i interface index
   * \param address to add
   */
  bool AddAddress (uint32_t i, Ipv6InterfaceAddress address);

  /**
   * \brief Get an address.
   * \param interfaceIndex interface index
   * \param addressIndex address index on the interface
   * \return Ipv6InterfaceAddress or assert if not found
   */
  Ipv6InterfaceAddress GetAddress (uint32_t interfaceIndex, uint32_t addressIndex) const;

  /**
   * \brief Get number of address for an interface.
   * \param interface interface index
   * \return number of address
   */
  uint32_t GetNAddresses (uint32_t interface) const;

  /**
   * \brief Remove an address from an interface.
   * \param interfaceIndex interface index
   * \param addressIndex address index on the interface
   */
  bool RemoveAddress (uint32_t interfaceIndex, uint32_t addressIndex);

  /**
   * \brief Set metric for an interface.
   * \param i index
   * \param metric
   */
  void SetMetric (uint32_t i, uint16_t metric);

  /**
   * \brief Get metric for an interface.
   * \param i index
   * \return metric
   */
  uint16_t GetMetric (uint32_t i) const;

  /**
   * \brief Get MTU for an interface.
   * \param i index
   * \return MTU
   */
  uint16_t GetMtu (uint32_t i) const;

  /**
   * \brief Is specified interface up ?
   * \param i interface index
   */
  bool IsUp (uint32_t i) const;

  /**
   * \brief Set an interface up.
   * \param i interface index
   */
  void SetUp (uint32_t i);

  /**
   * \brief set an interface down.
   * \param i interface index
   */
  void SetDown (uint32_t i);

  /**
   * \brief Is interface allows forwarding ?
   * \param i interface index
   */
  bool IsForwarding (uint32_t i) const;

  /**
   * \brief Enable or disable forwarding on interface
   * \param i interface index
   * \param val true = enable forwarding, false = disable
   */
  void SetForwarding (uint32_t i, bool val);

  /**
   * \brief Get device by index.
   * \param i device index on this stack
   * \return NetDevice pointer
   */
  Ptr<NetDevice> GetNetDevice (uint32_t i);

  /**
   * \brief Get ICMPv6 protocol.
   * \return Icmpv6L4Protocol pointer
   */
  Ptr<Icmpv6L4Protocol> GetIcmpv6 () const;

  /**
   * \brief Add an autoconfigured address with RA information.
   * \param interface interface index
   * \param network network prefix
   * \param mask network mask
   * \param flags flags of the prefix information option (home agent, ...)
   * \param validTime valid time of the prefix
   * \param preferredTime preferred time of the prefix
   * \param defaultRouter default router address
   */
  void AddAutoconfiguredAddress (uint32_t interface, Ipv6Address network, Ipv6Prefix mask, uint8_t flags, uint32_t validTime, uint32_t preferredTime, Ipv6Address defaultRouter = Ipv6Address::GetZero ());

  /**
   * \brief Remove an autoconfigured address.
   *
   * Typically it is used when an autoconfigured address expires.
   * \param interface interface index
   * \param network network prefix
   * \param mask network mask
   * \param defaultRouter gateway
   */
  void RemoveAutoconfiguredAddress (uint32_t interface, Ipv6Address network, Ipv6Prefix mask, Ipv6Address defaultRouter);

  /**
   * \brief Register the IPv6 Extensions.
   */
  virtual void RegisterExtensions ();

  /**
   * \brief Register the IPv6 Options.
   */
  virtual void RegisterOptions ();

protected:
  /**
   * \brief Dispose object.
   */
  virtual void DoDispose ();

  /**
   * \brief Notify other components connected to the node that a new stack member is now connected.
   *
   * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
   */
  virtual void NotifyNewAggregate ();

private:
  /* for unit-tests */
  friend class Ipv6L3ProtocolTestCase;
  friend class Ipv6ExtensionLooseRouting;

  typedef std::list<Ptr<Ipv6Interface> > Ipv6InterfaceList;
  typedef std::list<Ptr<Ipv6RawSocketImpl> > SocketList;
  typedef std::list<Ptr<IpL4Protocol> > L4List_t;

  typedef std::list< Ptr<Ipv6AutoconfiguredPrefix> > Ipv6AutoconfiguredPrefixList;
  typedef std::list< Ptr<Ipv6AutoconfiguredPrefix> >::iterator Ipv6AutoconfiguredPrefixListI;

  /**
   * \brief Callback to trace TX (transmission) packets.
   */
  TracedCallback<Ptr<const Packet>, Ptr<Ipv6>, uint32_t> m_txTrace;

  /**
   * \brief Callback to trace RX (reception) packets.
   */
  TracedCallback<Ptr<const Packet>, Ptr<Ipv6>, uint32_t> m_rxTrace;

  /**
   * \brief Callback to trace drop packets.
   */
  TracedCallback<const Ipv6Header &, Ptr<const Packet>, DropReason, Ptr<Ipv6>, uint32_t> m_dropTrace;

  /**
   * \brief Copy constructor.
   * \param o object to copy
   */
  Ipv6L3Protocol (const Ipv6L3Protocol& o);

  /**
   * \brief Copy constructor.
   * \param o object to copy
   */
  Ipv6L3Protocol &operator = (const Ipv6L3Protocol& o);

  /**
   * \brief Construct an IPv6 header.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param protocol L4 protocol
   * \param payloadSize payload size
   * \param hopLimit Hop limit
   * \return newly created IPv6 header
   */
  Ipv6Header BuildHeader (Ipv6Address src, Ipv6Address dst, uint8_t protocol,
                          uint16_t payloadSize, uint8_t hopLimit, uint8_t tclass);

  /**
   * \brief Send packet with route.
   * \param route route
   * \param packet packet to send
   * \param ipHeader IPv6 header to add to the packet
   */
  void SendRealOut (Ptr<Ipv6Route> route, Ptr<Packet> packet, Ipv6Header const& ipHeader);

  /**
   * \brief Forward a packet.
   * \param rtentry route
   * \param p packet to forward
   * \param header IPv6 header to add to the packet
   */
  void IpForward (Ptr<Ipv6Route> rtentry, Ptr<const Packet> p, const Ipv6Header& header);

  /**
   * \brief Forward a packet in multicast.
   * \param mrtentry route
   * \param p packet to forward
   * \param header IPv6 header to add to the packet
   */
  void IpMulticastForward (Ptr<Ipv6MulticastRoute> mrtentry, Ptr<const Packet> p, const Ipv6Header& header);

  /**
   * \brief Deliver a packet.
   * \param p packet delivered
   * \param ip IPv6 header
   * \param iif input interface packet was received
   */
  void LocalDeliver (Ptr<const Packet> p, Ipv6Header const& ip, uint32_t iif);

  /**
   * \brief Fallback when no route is found.
   * \param p packet
   * \param ipHeader IPv6 header
   * \param sockErrno error number
   */
  void RouteInputError (Ptr<const Packet> p, const Ipv6Header& ipHeader, Socket::SocketErrno sockErrno);

  /**
   * \brief Add an IPv6 interface to the stack.
   * \param interface interface to add
   * \return index of newly added interface
   */
  uint32_t AddIpv6Interface (Ptr<Ipv6Interface> interface);

  /**
   * \brief Setup loopback interface.
   */
  void SetupLoopback ();

  /**
   * \brief Set IPv6 forwarding state.
   * \param forward IPv6 forwarding enabled or not
   */
  virtual void SetIpForward (bool forward);

  /**
   * \brief Get IPv6 forwarding state.
   * \return forwarding state (enabled or not)
   */
  virtual bool GetIpForward () const;

  /**
   * \brief Set the ICMPv6 Redirect sending state.
   * \param sendIcmpv6Redirect ICMPv6 Redirect sending enabled or not
   */
  virtual void SetSendIcmpv6Redirect (bool sendIcmpv6Redirect);

  /**
   * \brief Get the ICMPv6 Redirect sending state.
   * \return ICMPv6 Redirect sending state (enabled or not)
   */
  virtual bool GetSendIcmpv6Redirect () const;

  /**
   * \brief Node attached to stack.
   */
  Ptr<Node> m_node;

  /**
   * \brief Forwarding packets (i.e. router mode) state.
   */
  bool m_ipForward;

  /**
   * \brief List of transport protocol.
   */
  L4List_t m_protocols;

  /**
   * \brief List of IPv6 interfaces.
   */
  Ipv6InterfaceList m_interfaces;

  /**
   * \brief Number of IPv6 interfaces managed by the stack.
   */
  uint32_t m_nInterfaces;

  /**
   * \brief Default TTL for outgoing packets.
   */
  uint8_t m_defaultTtl;

  /**
   * \brief Default TCLASS for outgoing packets.
   */
  uint8_t m_defaultTclass;

  /**
   * \brief Routing protocol.
   */
  Ptr<Ipv6RoutingProtocol> m_routingProtocol;

  /**
   * \brief List of IPv6 raw sockets.
   */
  SocketList m_sockets;

  /**
   * \brief List of IPv6 prefix received from RA.
   */
  Ipv6AutoconfiguredPrefixList m_prefixes;

  /**
   * \brief Allow ICMPv6 Redirect sending state
   */
  bool m_sendIcmpv6Redirect;
};

} /* namespace ns3 */

#endif /* IPV6_L3_PROTOCOL_H */

