/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation, INRIA
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
 * Authors: George F. Riley<riley@ece.gatech.edu>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef NODE_H
#define NODE_H

#include <vector>

#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"

namespace ns3 {

class Application;
class Packet;
class Address;


/**
 * \ingroup network
 *
 * \brief A network Node.
 *
 * This class holds together:
 *   - a list of NetDevice objects which represent the network interfaces
 *     of this node which are connected to other Node instances through
 *     Channel instances.
 *   - a list of Application objects which represent the userspace
 *     traffic generation applications which interact with the Node
 *     through the Socket API.
 *   - a node Id: a unique per-node identifier.
 *   - a system Id: a unique Id used for parallel simulations.
 *
 * Every Node created is added to the NodeList automatically.
 */
class Node : public Object
{
public:
  static TypeId GetTypeId (void);

  Node();
  /**
   * \param systemId a unique integer used for parallel simulations.
   */
  Node(uint32_t systemId);

  virtual ~Node();

  /**
   * \returns the unique id of this node.
   * 
   * This unique id happens to be also the index of the Node into
   * the NodeList. 
   */
  uint32_t GetId (void) const;

  /**
   * \returns the system id for parallel simulations associated
   *          to this node.
   */
  uint32_t GetSystemId (void) const;

  /**
   * \param device NetDevice to associate to this node.
   * \returns the index of the NetDevice into the Node's list of
   *          NetDevice.
   *
   * Associate this device to this node.
   */
  uint32_t AddDevice (Ptr<NetDevice> device);
  /**
   * \param index the index of the requested NetDevice
   * \returns the requested NetDevice associated to this Node.
   *
   * The indexes used by the GetDevice method start at one and
   * end at GetNDevices ()
   */
  Ptr<NetDevice> GetDevice (uint32_t index) const;
  /**
   * \returns the number of NetDevice instances associated
   *          to this Node.
   */
  uint32_t GetNDevices (void) const;

  /**
   * \param application Application to associate to this node.
   * \returns the index of the Application within the Node's list
   *          of Application.
   *
   * Associated this Application to this Node. 
   */
  uint32_t AddApplication (Ptr<Application> application);
  /**
   * \param application Application to remove from this node.
   *
   * Remove this Application from this Node. 
   */
  void DeleteApplication (Ptr<Application> application);
  /**
   * \param index
   * \returns the application associated to this requested index
   *          within this Node.
   */
  Ptr<Application> GetApplication (uint32_t index) const;

  /**
   * \returns the number of applications associated to this Node.
   */
  uint32_t GetNApplications (void) const;

  /**
   * A protocol handler
   *
   * \param device a pointer to the net device which received the packet
   * \param packet the packet received
   * \param protocol the 16 bit protocol number associated with this packet.
   *        This protocol number is expected to be the same protocol number
   *        given to the Send method by the user on the sender side.
   * \param sender the address of the sender
   * \param receiver the address of the receiver; Note: this value is
   *                 only valid for promiscuous mode protocol
   *                 handlers.  Note:  If the L2 protocol does not use L2
   *                 addresses, the address reported here is the value of 
   *                 device->GetAddress().
   * \param packetType type of packet received
   *                   (broadcast/multicast/unicast/otherhost); Note:
   *                   this value is only valid for promiscuous mode
   *                   protocol handlers.
   */
  typedef Callback<void,Ptr<NetDevice>, Ptr<const Packet>,uint16_t,const Address &,
                   const Address &, NetDevice::PacketType> ProtocolHandler;
  /**
   * \param handler the handler to register
   * \param protocolType the type of protocol this handler is 
   *        interested in. This protocol type is a so-called
   *        EtherType, as registered here:
   *        http://standards.ieee.org/regauth/ethertype/eth.txt
   *        the value zero is interpreted as matching all
   *        protocols.
   * \param device the device attached to this handler. If the
   *        value is zero, the handler is attached to all
   *        devices on this node.
   * \param promiscuous whether to register a promiscuous mode handler
   */
  void RegisterProtocolHandler (ProtocolHandler handler, 
                                uint16_t protocolType,
                                Ptr<NetDevice> device,
                                bool promiscuous=false);
  /**
   * \param handler the handler to unregister
   *
   * After this call returns, the input handler will never
   * be invoked anymore.
   */
  void UnregisterProtocolHandler (ProtocolHandler handler);

  /**
   * A callback invoked whenever a device is added to a node.
   */
  typedef Callback<void,Ptr<NetDevice> > DeviceAdditionListener;
  /**
   * \param listener the listener to add
   *
   * Add a new listener to the list of listeners for the device-added
   * event. When a new listener is added, it is notified of the existance
   * of all already-added devices to make discovery of devices easier.
   */
  void RegisterDeviceAdditionListener (DeviceAdditionListener listener);
  /**
   * \param listener the listener to remove
   *
   * Remove an existing listener from the list of listeners for the 
   * device-added event.
   */
  void UnregisterDeviceAdditionListener (DeviceAdditionListener listener);



  /**
   * \returns true if checksums are enabled, false otherwise.
   */
  static bool ChecksumEnabled (void);

  //yibo
  uint32_t GetNodeType();
  
  /*
  bool CheckIngressAdmission(uint32_t port,uint32_t qIndex, uint32_t psize);
  bool CheckEgressAdmission(uint32_t port,uint32_t qIndex, uint32_t psize);
  void UpdateIngressAdmission(uint32_t port,uint32_t qIndex, uint32_t psize);
  void UpdateEgressAdmission(uint32_t port,uint32_t qIndex, uint32_t psize);
  void RemoveFromIngressAdmission(uint32_t port,uint32_t qIndex, uint32_t psize);
  void RemoveFromEgressAdmission(uint32_t port,uint32_t qIndex, uint32_t psize);

  void GetPauseClasses(uint32_t port, uint32_t qIndex, bool pClasses[]);
  
  void SetBroadcomParams(uint32_t maxBufferBytes, 
	  uint32_t maxIngressPortBytes,
	  uint32_t maxIngressSPBytes,
	  uint32_t maxIngressPGBytes,
	  uint32_t maxEgressPortBytes,
	  uint32_t maxEgressSPBytes,
	  uint32_t maxEgressPGBytes,
	  uint32_t buffer_cell_limit_sp, //ingress sp buffer threshold p.120
	  uint32_t buffer_cell_limit_sp_shared, //ingress sp buffer shared threshold p.120, nonshare -> share
	  uint32_t pg_min_cell, //ingress pg guarantee p.121					---1
	  uint32_t port_min_cell, //ingress port guarantee						---2
	  uint32_t pg_shared_limit_cell, //max buffer for an ingress pg			---3	PAUSE
	  uint32_t port_max_shared_cell, //max buffer for an ingress port		---4	PAUSE
	  uint32_t pg_hdrm_limit, //ingress pg headroom
	  uint32_t port_max_pkt_size, //ingress global headroom
	  uint32_t op_buffer_shared_limit_cell, //per egress sp limit
	  uint32_t m_uc_port_config_cell //per egress port limit
	  );
	  */
protected:
	//yibo
	//uint32_t GetIngressSP(uint32_t port,uint32_t pgIndex);
	//uint32_t GetEgressSP(uint32_t port,uint32_t qIndex);


  /**
   * The dispose method. Subclasses must override this method
   * and must chain up to it by calling Node::DoDispose at the
   * end of their own DoDispose method.
   */
  virtual void DoDispose (void);
  virtual void DoStart (void);
protected:

	//yibo
	uint32_t m_node_type;
	/*
	uint32_t m_maxBufferBytes;
	uint32_t m_usedTotalBytes;

	uint32_t m_usedIngressPGMinBytes[32][8];
	uint32_t m_usedIngressPortMinBytes[32];
	uint32_t m_usedIngressSPBytes[4];
	uint32_t m_usedIngressSPSharedBytes;
	uint32_t m_usedIngressPGSharedBytes[32][8];
	uint32_t m_usedIngressPortSharedBytes[32];

	uint32_t m_usedIngressPGMinExceed[32][8];
	uint32_t m_usedIngressPortMinExceed[32];
	uint32_t m_usedIngressSPExceed[4];
	uint32_t m_usedIngressSPSharedExceed;
	uint32_t m_usedIngressPGSharedExceed[32][8];
	uint32_t m_usedIngressPortSharedExceed[32];

	uint32_t m_usedIngressHeadroomBytes;

	

	
	uint32_t m_usedEgressQMinBytes[32][8];
	uint32_t m_usedEgressQSharedBytes[32][8];
	uint32_t m_usedEgressPortBytes[32];
	uint32_t m_usedEgressSPBytes[4];
	
	

	//ingress params
	uint32_t m_buffer_cell_limit_sp; //ingress sp buffer threshold p.120
	uint32_t m_buffer_cell_limit_sp_shared; //ingress sp buffer shared threshold p.120, nonshare -> share
	uint32_t m_pg_min_cell; //ingress pg guarantee p.121					---1
	uint32_t m_port_min_cell; //ingress port guarantee						---2
	uint32_t m_pg_shared_limit_cell; //max buffer for an ingress pg			---3	PAUSE
	uint32_t m_port_max_shared_cell; //max buffer for an ingress port		---4	PAUSE
	uint32_t m_pg_hdrm_limit; //ingress pg headroom
	uint32_t m_port_max_pkt_size; //ingress global headroom
	//still needs reset limits..

	//egress params
	uint32_t m_q_min_cell;	//egress queue guaranteed buffer
	uint32_t m_op_uc_port_config1_cell; //egress queue threshold
	uint32_t m_op_uc_port_config_cell; //egress port threshold
	uint32_t m_op_buffer_shared_limit_cell; //egress sp threshold

	*/

  void NotifyDeviceAdded (Ptr<NetDevice> device);
  bool NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet>, uint16_t protocol, const Address &from);
  bool PromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet>, uint16_t protocol,
                                 const Address &from, const Address &to, NetDevice::PacketType packetType);
  bool ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet>, uint16_t protocol,
                          const Address &from, const Address &to, NetDevice::PacketType packetType, bool promisc);

  void Construct (void);

  struct ProtocolHandlerEntry {
    ProtocolHandler handler;
    Ptr<NetDevice> device;
    uint16_t protocol;
    bool promiscuous;
  };
  typedef std::vector<struct Node::ProtocolHandlerEntry> ProtocolHandlerList;
  typedef std::vector<DeviceAdditionListener> DeviceAdditionListenerList;

  uint32_t    m_id;         // Node id for this node
  uint32_t    m_sid;        // System id for this node
  std::vector<Ptr<NetDevice> > m_devices;
  std::vector<Ptr<Application> > m_applications;
  ProtocolHandlerList m_handlers;
  DeviceAdditionListenerList m_deviceAdditionListeners;

  // Yuliang
public:
  virtual bool SwitchReceiveFromDevice(Ptr<NetDevice> device, Ptr<Packet> packet, CustomHeader &ch);
  virtual void SwitchNotifyDequeue(uint32_t ifIndex, uint32_t qIndex, Ptr<Packet> p);
};

} // namespace ns3

#endif /* NODE_H */
