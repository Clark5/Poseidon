/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation
 *               2007 INRIA
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

#ifndef NS3_SOCKET_H
#define NS3_SOCKET_H

#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/tag.h"
#include "ns3/object.h"
#include "ns3/net-device.h"
#include "address.h"
#include <stdint.h>
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/nstime.h"

namespace ns3 {


class Node;
class Packet;

/**
 * \ingroup network
 * \defgroup socket Socket
 */

/**
 * \brief A low-level Socket API based loosely on the BSD Socket API.
 * \ingroup socket
 *
 * A few things to keep in mind about this type of socket:
 * - it uses ns-3 API constructs such as class ns3::Address instead of
 *   C-style structs
 * - in contrast to the original BSD socket API, this API is asynchronous:
 *   it does not contain blocking calls.  Sending and receiving operations
 *   must make use of the callbacks provided.
 * - It also uses class ns3::Packet as a fancy byte buffer, allowing
 *   data to be passed across the API using an ns-3 Packet instead of
 *   a raw data pointer.
 * - Not all of the full POSIX sockets API is supported
 *
 * Other than that, it tries to stick to the BSD API to make it
 * easier for those who know the BSD API to use this API.
 * More details are provided in the ns-3 tutorial.
 */
class Socket : public Object
{
public:
  static TypeId GetTypeId (void);

  Socket (void);
  virtual ~Socket (void);

  enum SocketErrno {
    ERROR_NOTERROR,
    ERROR_ISCONN,
    ERROR_NOTCONN,
    ERROR_MSGSIZE,
    ERROR_AGAIN,
    ERROR_SHUTDOWN,
    ERROR_OPNOTSUPP,
    ERROR_AFNOSUPPORT,
    ERROR_INVAL,
    ERROR_BADF,
    ERROR_NOROUTETOHOST,
    ERROR_NODEV,
    ERROR_ADDRNOTAVAIL,
    ERROR_ADDRINUSE,
    SOCKET_ERRNO_LAST
  };

  enum SocketType {
    NS3_SOCK_STREAM,
    NS3_SOCK_SEQPACKET,
    NS3_SOCK_DGRAM,
    NS3_SOCK_RAW
  };

  /**
   * This method wraps the creation of sockets that is performed
   * on a given node by a SocketFactory specified by TypeId.
   *
   * \return A smart pointer to a newly created socket.
   *
   * \param node The node on which to create the socket
   * \param tid The TypeId of a SocketFactory class to use
   */
  static Ptr<Socket> CreateSocket (Ptr<Node> node, TypeId tid);
  /**
   * \return the errno associated to the last call which failed in this
   *         socket. Each socket's errno is initialized to zero
   *         when the socket is created.
   */
  virtual enum Socket::SocketErrno GetErrno (void) const = 0;
  /**
    * \return the socket type, analogous to getsockopt (SO_TYPE)
    */
  virtual enum Socket::SocketType GetSocketType (void) const = 0;
  /**
   * \returns the node this socket is associated with.
   */
  virtual Ptr<Node> GetNode (void) const = 0;
  /**
   * \brief Specify callbacks to allow the caller to determine if
   * the connection succeeds of fails.
   * \param connectionSucceeded this callback is invoked when the
   *        connection request initiated by the user is successfully
   *        completed. The callback is passed  back a pointer to
   *        the same socket object.
   * \param connectionFailed this callback is invoked when the
   *        connection request initiated by the user is unsuccessfully
   *        completed. The callback is passed back a pointer to the
   *        same socket object.
   */
  void SetConnectCallback (Callback<void, Ptr<Socket> > connectionSucceeded,
                           Callback<void,  Ptr<Socket> > connectionFailed);
  /**
   * \brief Detect socket recv() events such as graceful shutdown or error.
   *
   * For connection-oriented sockets, the first callback is used to signal
   * that the remote side has gracefully shut down the connection, and the
   * second callback denotes an error corresponding to cases in which
   * a traditional recv() socket call might return -1 (error), such
   * as a connection reset.  For datagram sockets, these callbacks may
   * never be invoked.
   *
   * \param normalClose this callback is invoked when the
   *        peer closes the connection gracefully
   * \param errorClose this callback is invoked when the
   *        connection closes abnormally
   */
  void SetCloseCallbacks (Callback<void, Ptr<Socket> > normalClose,
                          Callback<void, Ptr<Socket> > errorClose);
  /**
   * \brief Accept connection requests from remote hosts
   * \param connectionRequest Callback for connection request from peer.
   *        This user callback is passed a pointer to this socket, the
   *        ip address and the port number of the connection originator.
   *        This callback must return true to accept the incoming connection,
   *        false otherwise. If the connection is accepted, the
   *        "newConnectionCreated" callback will be invoked later to
   *        give access to the user to the socket created to match
   *        this new connection. If the user does not explicitly
   *        specify this callback, all incoming  connections will be refused.
   * \param newConnectionCreated Callback for new connection: when a new
   *        is accepted, it is created and the corresponding socket is passed
   *        back to the user through this callback. This user callback is
   *        passed a pointer to the new socket, and the ip address and
   *        port number of the connection originator.
   */
  void SetAcceptCallback (Callback<bool, Ptr<Socket>,
                                   const Address &> connectionRequest,
                          Callback<void, Ptr<Socket>,
                                   const Address&> newConnectionCreated);
  /**
   * \brief Notify application when a packet has been sent from transport
   *        protocol (non-standard socket call)
   * \param dataSent Callback for the event that data is sent from the
   *        underlying transport protocol.  This callback is passed a
   *        pointer to the socket, and the number of bytes sent.
   */
  void SetDataSentCallback (Callback<void, Ptr<Socket>,
                                     uint32_t> dataSent);
  /**
   * \brief Notify application when space in transmit buffer is added
   *
   *        This callback is intended to notify a
   *        socket that would have been blocked in a blocking socket model
   *        that space is available in the transmit buffer and that it
   *        can call Send() again.
   *
   * \param sendCb Callback for the event that the socket transmit buffer
   *        fill level has decreased.  This callback is passed a pointer to
   *        the socket, and the number of bytes available for writing
   *        into the buffer (an absolute value).  If there is no transmit
   *        buffer limit, a maximum-sized integer is always returned.
   */
  void SetSendCallback (Callback<void, Ptr<Socket>, uint32_t> sendCb);
  /**
   * \brief Notify application when new data is available to be read.
   *
   *        This callback is intended to notify a socket that would
   *        have been blocked in a blocking socket model that data
   *        is available to be read.
   */
  void SetRecvCallback (Callback<void, Ptr<Socket> >);
  /**
   * \brief Allocate a local endpoint for this socket.
   * \param address the address to try to allocate
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind (const Address &address) = 0;

  /**
   * \brief Allocate a local IPv4 endpoint for this socket.
   *
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind () = 0;

  /**
   * \brief Allocate a local IPv6 endpoint for this socket.
   *
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind6 () = 0;

  /**
   * \brief Close a socket.
   * \returns zero on success, -1 on failure.
   *
   * After the Close call, the socket is no longer valid, and cannot
   * safely be used for subsequent operations.
   */
  virtual int Close (void) = 0;

  /**
   * \returns zero on success, -1 on failure.
   *
   * Do not allow any further Send calls. This method is typically
   * implemented for Tcp sockets by a half close.
   */
  virtual int ShutdownSend (void) = 0;

  /**
   * \returns zero on success, -1 on failure.
   *
   * Do not allow any further Recv calls. This method is typically
   * implemented for Tcp sockets by a half close.
   */
  virtual int ShutdownRecv (void) = 0;

  /**
   * \brief Initiate a connection to a remote host
   * \param address Address of remote.
   */
  virtual int Connect (const Address &address) = 0;

  /**
   * \brief Listen for incoming connections.
   * \returns 0 on success, -1 on error (in which case errno is set).
   */
  virtual int Listen (void) = 0;

  /**
   * \brief Returns the number of bytes which can be sent in a single call
   * to Send.
   *
   * For datagram sockets, this returns the number of bytes that
   * can be passed atomically through the underlying protocol.
   *
   * For stream sockets, this returns the available space in bytes
   * left in the transmit buffer.
   */
  virtual uint32_t GetTxAvailable (void) const = 0;

  /**
   * \brief Send data (or dummy data) to the remote host
   *
   * This function matches closely in semantics to the send() function
   * call in the standard C library (libc):
   *   ssize_t send (int s, const void *msg, size_t len, int flags);
   * except that the send I/O is asynchronous.  This is the
   * primary Send method at this low-level API and must be implemented
   * by subclasses.
   *
   * In a typical blocking sockets model, this call would block upon
   * lack of space to hold the message to be sent.  In ns-3 at this
   * API, the call returns immediately in such a case, but the callback
   * registered with SetSendCallback() is invoked when the socket
   * has space (when it conceptually unblocks); this is an asynchronous
   * I/O model for send().
   *
   * This variant of Send() uses class ns3::Packet to encapsulate
   * data, rather than providing a raw pointer and length field.
   * This allows an ns-3 application to attach tags if desired (such
   * as a flow ID) and may allow the simulator to avoid some data
   * copies.  Despite the appearance of sending Packets on a stream
   * socket, just think of it as a fancy byte buffer with streaming
   * semantics.
   *
   * If either the message buffer within the Packet is too long to pass
   * atomically through the underlying protocol (for datagram sockets),
   * or the message buffer cannot entirely fit in the transmit buffer
   * (for stream sockets), -1 is returned and SocketErrno is set
   * to ERROR_MSGSIZE.  If the packet does not fit, the caller can
   * split the Packet (based on information obtained from
   * GetTxAvailable) and reattempt to send the data.
   *
   * The flags argument is formed by or'ing one or more of the values:
   *        MSG_OOB        process out-of-band data
   *        MSG_DONTROUTE  bypass routing, use direct interface
   * These flags are _unsupported_ as of ns-3.1.
   *
   * \param p ns3::Packet to send
   * \param flags Socket control flags
   * \returns the number of bytes accepted for transmission if no error
   *          occurs, and -1 otherwise.
   *
   * \see SetSendCallback
   */
  virtual int Send (Ptr<Packet> p, uint32_t flags) = 0;

  /**
   * \brief Send data to a specified peer.
   *
   * This method has similar semantics to Send () but subclasses may
   * want to provide checks on socket state, so the implementation is
   * pushed to subclasses.
   *
   * \param p packet to send
   * \param flags Socket control flags
   * \param toAddress IP Address of remote host
   * \returns -1 in case of error or the number of bytes copied in the
   *          internal buffer and accepted for transmission.
   */
  virtual int SendTo (Ptr<Packet> p, uint32_t flags,
                      const Address &toAddress) = 0;

  /**
   * Return number of bytes which can be returned from one or
   * multiple calls to Recv.
   * Must be possible to call this method from the Recv callback.
   */
  virtual uint32_t GetRxAvailable (void) const = 0;

  /**
   * \brief Read data from the socket
   *
   * This function matches closely in semantics to the recv() function
   * call in the standard C library (libc):
   *   ssize_t recv (int s, void *buf, size_t len, int flags);
   * except that the receive I/O is asynchronous.  This is the
   * primary Recv method at this low-level API and must be implemented
   * by subclasses.
   *
   * This method is normally used only on a connected socket.
   * In a typical blocking sockets model, this call would block until
   * at least one byte is returned or the connection closes.
   * In ns-3 at this API, the call returns immediately in such a case
   * and returns 0 if nothing is available to be read.
   * However, an application can set a callback, ns3::SetRecvCallback,
   * to be notified of data being available to be read
   * (when it conceptually unblocks); this is an asynchronous
   * I/O model for recv().
   *
   * This variant of Recv() uses class ns3::Packet to encapsulate
   * data, rather than providing a raw pointer and length field.
   * This allows an ns-3 application to attach tags if desired (such
   * as a flow ID) and may allow the simulator to avoid some data
   * copies.  Despite the appearance of receiving Packets on a stream
   * socket, just think of it as a fancy byte buffer with streaming
   * semantics.
   *
   * The semantics depend on the type of socket.  For a datagram socket,
   * each Recv() returns the data from at most one Send(), and order
   * is not necessarily preserved.  For a stream socket, the bytes
   * are delivered in order, and on-the-wire packet boundaries are
   * not preserved.
   *
   * The flags argument is formed by or'ing one or more of the values:
   *        MSG_OOB             process out-of-band data
   *        MSG_PEEK            peek at incoming message
   * None of these flags are supported for now.
   *
   * Some variants of Recv() are supported as additional API,
   * including RecvFrom(), overloaded Recv() without arguments,
   * and variants that use raw character buffers.
   *
   * \param maxSize reader will accept packet up to maxSize
   * \param flags Socket control flags
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet conforming
   * to the maxSize and flags.
   *
   * \see SetRecvCallback
   */
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags) = 0;

  /**
   * \brief Read a single packet from the socket and retrieve the sender
   * address.
   *
   * Calls Recv(maxSize, flags) with maxSize
   * implicitly set to maximum sized integer, and flags set to zero.
   *
   * This method has similar semantics to Recv () but subclasses may
   * want to provide checks on socket state, so the implementation is
   * pushed to subclasses.
   *
   * \param maxSize reader will accept packet up to maxSize
   * \param flags Socket control flags
   * \param fromAddress output parameter that will return the
   * address of the sender of the received packet, if any.  Remains
   * untouched if no packet is received.
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet.
   */
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags,
                                Address &fromAddress) = 0;

                                
  //yibo: to get local port
  virtual uint32_t GetLocalPort()
  {
	  std::cout<<"WARNING: shouldn't reach here -- socket.h\n";
	  return 0;
  }
                                
  /////////////////////////////////////////////////////////////////////
  //   The remainder of these public methods are overloaded methods  //
  //   or variants of Send() and Recv(), and they are non-virtual    //
  /////////////////////////////////////////////////////////////////////

  /**
   * \brief Send data (or dummy data) to the remote host
   *
   * Overloaded version of Send(..., flags) with flags set to zero.
   *
   * \param p ns3::Packet to send
   * \returns the number of bytes accepted for transmission if no error
   *          occurs, and -1 otherwise.
   */
  int Send (Ptr<Packet> p);

  /**
   * \brief Send data (or dummy data) to the remote host
   *
   * This method is provided so as to have an API which is closer in
   * appearance to that of real network or BSD sockets.
   *
   * \param buf A pointer to a raw byte buffer of some data to send.  If
   * this buffer is 0, we send dummy data whose size is specified by the
   * second parameter
   * \param size the number of bytes to copy from the buffer
   * \param flags Socket control flags
   */
  int Send (const uint8_t* buf, uint32_t size, uint32_t flags);


  /**
   * \brief Send data to a specified peer.
   *
   * This method is provided so as to have an API which is closer in
   * appearance to that of real network or BSD sockets.
   *
   * \param buf A pointer to a raw byte buffer of some data to send.
   * If this is 0, we send dummy data whose size is specified by the
   * third parameter
   * \param size the number of bytes to copy from the buffer
   * \param flags Socket control flags
   * \param address IP Address of remote host
   * \returns -1 in case of error or the number of bytes copied in the
   *          internal buffer and accepted for transmission.
   *
   */
  int SendTo (const uint8_t* buf, uint32_t size, uint32_t flags,
              const Address &address);

  /**
   * \brief Read a single packet from the socket
   *
   * Overloaded version of Recv(maxSize, flags) with maxSize
   * implicitly set to maximum sized integer, and flags set to zero.
   *
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet.
   */
  Ptr<Packet> Recv (void);

  /**
   * \brief Recv data (or dummy data) from the remote host
   *
   * This method is provided so as to have an API which is closer in
   * appearance to that of real network or BSD sockets.
   *
   * If the underlying packet was carring null (fake) data, this buffer
   * will be zeroed up to the length specified by the return value.
   *
   * \param buf A pointer to a raw byte buffer to write the data to.
   * \param size Number of bytes (at most) to copy to buf
   * \param flags any flags to pass to the socket
   * \returns number of bytes copied into buf
   */
  int Recv (uint8_t* buf, uint32_t size, uint32_t flags);

  /**
   * \brief Read a single packet from the socket and retrieve the sender
   * address.
   *
   * Calls RecvFrom (maxSize, flags, fromAddress) with maxSize
   * implicitly set to maximum sized integer, and flags set to zero.
   *
   * \param fromAddress output parameter that will return the
   * address of the sender of the received packet, if any.  Remains
   * untouched if no packet is received.
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet.
   */
  Ptr<Packet> RecvFrom (Address &fromAddress);

  /**
   * \brief Read a single packet from the socket and retrieve the sender
   * address.
   *
   * This method is provided so as to have an API which is closer in
   * appearance to that of real network or BSD sockets.
   *
   * \param buf A pointer to a raw byte buffer to write the data to.
   * If the underlying packet was carring null (fake) data, this buffer
   * will be zeroed up to the length specified by the return value.
   * \param size Number of bytes (at most) to copy to buf
   * \param flags any flags to pass to the socket
   * \param fromAddress output parameter that will return the
   * address of the sender of the received packet, if any.  Remains
   * untouched if no packet is received.
   * \returns number of bytes copied into buf
   */
  int RecvFrom (uint8_t* buf, uint32_t size, uint32_t flags,
                Address &fromAddress);
  /**
   * \param address the address name this socket is associated with.
   * \returns 0 if success, -1 otherwise
   */
  virtual int GetSockName (Address &address) const = 0;

  /**
   * \brief Bind a socket to specific device.
   *
   * This method corresponds to using setsockopt() SO_BINDTODEVICE
   * of real network or BSD sockets.   If set on a socket, this option will
   * force packets to leave the bound device regardless of the device that
   * IP routing would naturally choose.  In the receive direction, only
   * packets received from the bound interface will be delivered.
   *
   * This option has no particular relationship to binding sockets to
   * an address via Socket::Bind ().  It is possible to bind sockets to a
   * specific IP address on the bound interface by calling both
   * Socket::Bind (address) and Socket::BindToNetDevice (device), but it
   * is also possible to bind to mismatching device and address, even if
   * the socket can not receive any packets as a result.
   *
   * \param netdevice Pointer to Netdevice of desired interface
   * \returns nothing
   */
  virtual void BindToNetDevice (Ptr<NetDevice> netdevice);

  /**
   * \brief Returns socket's bound netdevice, if any.
   *
   * This method corresponds to using getsockopt() SO_BINDTODEVICE
   * of real network or BSD sockets.
   *
   *
   * \returns Pointer to interface.
   */
  Ptr<NetDevice> GetBoundNetDevice ();


  /**
   * \brief Configure whether broadcast datagram transmissions are allowed
   *
   * This method corresponds to using setsockopt() SO_BROADCAST of
   * real network or BSD sockets.  If set on a socket, this option
   * will enable or disable packets to be transmitted to broadcast
   * destination addresses.
   *
   * \param allowBroadcast Whether broadcast is allowed
   * \return true if operation succeeds
   */
  virtual bool SetAllowBroadcast (bool allowBroadcast) = 0;

  /**
   * \brief Query whether broadcast datagram transmissions are allowed
   *
   * This method corresponds to using getsockopt() SO_BROADCAST of
   * real network or BSD sockets.
   *
   * \returns true if broadcast is allowed, false otherwise
   */
  virtual bool GetAllowBroadcast () const = 0;

  /**
   * \brief Enable/Disable receive packet information to socket.
   *
   * For IP_PKTINFO/IP6_PKTINFO. This method is only usable for
   * Raw socket and Datagram Socket. Not supported for Stream socket.
   *
   * Method doesn't make distinction between IPv4 and IPv6. If it is enabled,
   * it is enabled for all types of sockets that supports packet information
   *
   * \param flag Enable/Disable receive information
   * \returns nothing
   */
  void SetRecvPktInfo (bool flag);

  /**
   * \brief Get status indicating whether enable/disable packet information to socket
   *
   * \returns True if packet information should be sent to socket
   */
  bool IsRecvPktInfo () const;

  /*
   * \brief Manually set IP Type of Service field
   *
   * This method corresponds to using setsockopt () IP_TOS of
   * real network or BSD sockets. This option is for IPv4 only.
   * Setting the IP TOS should also change the socket queueing
   * priority as stated in the man page. However, socket priority
   * is not yet supported.
   *
   * \param ipTos The desired TOS value for IP headers
   */
  void SetIpTos (uint8_t ipTos);

  /*
   * \brief Query the value of IP Type of Service of this socket
   *
   * This method corresponds to using getsockopt () IP_TOS of real network
   * or BSD sockets.
   *
   * \return The raw IP TOS value
   */
  uint8_t GetIpTos (void) const;

  /**
   * \brief Tells a socket to pass information about IP Type of Service up the stack
   *
   * This method corresponds to using setsockopt () IP_RECVTOS of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpTosTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv4RecvTos Whether the socket should add SocketIpv4TosTag tag
   * to the packet
   */
  void SetIpRecvTos (bool ipv4RecvTos);

  /**
   * \brief Ask if the socket is currently passing information about IP Type of Service up the stack
   *
   * This method corresponds to using getsockopt () IP_RECVTOS of real
   * network or BSD sockets.
   *
   * \return Wheter the IP_RECVTOS is set
   */
  bool IsIpRecvTos (void) const;

  /*
   * \brief Manually set IPv6 Traffic Class field
   *
   * This method corresponds to using setsockopt () IPV6_TCLASS of
   * real network or BSD sockets. This option is for IPv6 only.
   * Setting the IPV6_TCLASSS to -1 clears the option and let the socket
   * uses the default value.
   *
   * \param ipTclass The desired TCLASS value for IPv6 headers
   */
  void SetIpv6Tclass (int ipTclass);

  /*
   * \brief Query the value of IPv6 Traffic Class field of this socket
   *
   * This method corresponds to using getsockopt () IPV6_TCLASS of real network
   * or BSD sockets.
   *
   * \return The raw IPV6_TCLASS value
   */
  uint8_t GetIpv6Tclass (void) const;

  /**
   * \brief Tells a socket to pass information about IPv6 Traffic Class up the stack
   *
   * This method corresponds to using setsockopt () IPV6_RECVTCLASS of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpv6TclasssTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv6RecvTclass Whether the socket should add SocketIpv6TclassTag tag
   * to the packet
   */
  void SetIpv6RecvTclass (bool ipv6RecvTclass);

  /**
   * \brief Ask if the socket is currently passing information about IPv6 Traffic Class up the stack
   *
   * This method corresponds to using getsockopt () IPV6_RECVTCLASS of real
   * network or BSD sockets.
   *
   * \return Wheter the IPV6_RECVTCLASS is set
   */
  bool IsIpv6RecvTclass (void) const;

  /*
   * \brief Manually set IP Time to Live field
   *
   * This method corresponds to using setsockopt () IP_TTL of
   * real network or BSD sockets.
   *
   * \param ipTtl The desired TTL value for IP headers
   */
  virtual void SetIpTtl (uint8_t ipTtl);

  /*
   * \brief Query the value of IP Time to Live field of this socket
   *
   * This method corresponds to using getsockopt () IP_TTL of real network
   * or BSD sockets.
   *
   * \return The raw IP TTL value
   */
  virtual uint8_t GetIpTtl (void) const;

  /**
   * \brief Tells a socket to pass information about IP_TTL up the stack
   *
   * This method corresponds to using setsockopt () IP_RECVTTL of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpTtlTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv4RecvTtl Whether the socket should add SocketIpv4TtlTag tag
   * to the packet
   */
  void SetIpRecvTtl (bool ipv4RecvTtl);

  /**
   * \brief Ask if the socket is currently passing information about IP_TTL up the stack
   *
   * This method corresponds to using getsockopt () IP_RECVTTL of real
   * network or BSD sockets.
   *
   * \return Wheter the IP_RECVTTL is set
   */
  bool IsIpRecvTtl (void) const;

  /*
   * \brief Manually set IPv6 Hop Limit
   *
   * This method corresponds to using setsockopt () IPV6_HOPLIMIT of
   * real network or BSD sockets.
   *
   * \param ipHopLimit The desired Hop Limit value for IPv6 headers
   */
  virtual void SetIpv6HopLimit (uint8_t ipHopLimit);

  /*
   * \brief Query the value of IP Hop Limit field of this socket
   *
   * This method corresponds to using getsockopt () IPV6_HOPLIMIT of real network
   * or BSD sockets.
   *
   * \return The raw IPv6 Hop Limit value
   */
  virtual uint8_t GetIpv6HopLimit (void) const;

  /**
   * \brief Tells a socket to pass information about IPv6 Hop Limit up the stack
   *
   * This method corresponds to using setsockopt () IPV6_RECVHOPLIMIT of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpv6HopLimitTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv6RecvHopLimit Whether the socket should add SocketIpv6HopLimitTag tag
   * to the packet
   */
  void SetIpv6RecvHopLimit (bool ipv6RecvHopLimit);

  /**
   * \brief Ask if the socket is currently passing information about IPv6 Hop Limit up the stack
   *
   * This method corresponds to using getsockopt () IPV6_RECVHOPLIMIT of real
   * network or BSD sockets.
   *
   * \return Wheter the IPV6_RECVHOPLIMIT is set
   */
  bool IsIpv6RecvHopLimit (void) const;

  /**
   * Experimental functions to set/get Deadline that this socket should meet.
   * Socket classes that want to be deadline aware should override this functions.
   */
  virtual void SetDeadline (Time deadline);
  virtual Time GetDeadline (void) const;

  /**
   * Experimental functions to set/get number of bytes that this socket should transmit.
   * They are going to be used together with {Set, Get}Deadline functions.
   *
   * Socket classes that want to be deadline aware should override this functions.
   */
  virtual void SetBytesToTx (uint64_t bytes);
  virtual uint64_t GetBytesToTx (void) const;

protected:
  void NotifyConnectionSucceeded (void);
  void NotifyConnectionFailed (void);
  void NotifyNormalClose (void);
  void NotifyErrorClose (void);
  bool NotifyConnectionRequest (const Address &from);
  void NotifyNewConnectionCreated (Ptr<Socket> socket, const Address &from);
  void NotifyDataSent (uint32_t size);
  void NotifySend (uint32_t spaceAvailable);
  void NotifyDataRecv (void);
  virtual void DoDispose (void);

  bool IsManualIpTos (void) const;
  bool IsManualIpv6Tclass (void) const;
  bool IsManualIpTtl (void) const;
  bool IsManualIpv6HopLimit (void) const;

  Ptr<NetDevice> m_boundnetdevice;
  bool m_recvPktInfo;

private:
  Callback<void, Ptr<Socket> >                   m_connectionSucceeded;
  Callback<void, Ptr<Socket> >                   m_connectionFailed;
  Callback<void, Ptr<Socket> >                   m_normalClose;
  Callback<void, Ptr<Socket> >                   m_errorClose;
  Callback<bool, Ptr<Socket>, const Address &>   m_connectionRequest;
  Callback<void, Ptr<Socket>, const Address&>    m_newConnectionCreated;
  Callback<void, Ptr<Socket>, uint32_t>          m_dataSent;
  Callback<void, Ptr<Socket>, uint32_t >         m_sendCb;
  Callback<void, Ptr<Socket> >                   m_receivedData;

  //IPv4 options
  bool m_manualIpTos;
  bool m_manualIpTtl;
  bool m_ipRecvTos;
  bool m_ipRecvTtl;

  uint8_t m_ipTos;
  uint8_t m_ipTtl;

  //IPv6 options
  bool m_manualIpv6Tclass;
  bool m_manualIpv6HopLimit;
  bool m_ipv6RecvTclass;
  bool m_ipv6RecvHopLimit;

  uint8_t m_ipv6Tclass;
  uint8_t m_ipv6HopLimit;
};

/**
 * \brief This class implements a tag that carries an address
 * of a packet across the socket interface.
 */
class SocketAddressTag : public Tag
{
public:
  SocketAddressTag ();
  void SetAddress (Address addr);
  Address GetAddress (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  Address m_address;
};

/**
 * \brief This class implements a tag that carries the socket-specific
 * TTL of a packet to the IP layer
 */
class SocketIpTtlTag : public Tag
{
public:
  SocketIpTtlTag ();
  void SetTtl (uint8_t ttl);
  uint8_t GetTtl (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_ttl;
};

/**
 * \brief This class implements a tag that carries the socket-specific
 * HOPLIMIT of a packet to the IPv6 layer
 */
class SocketIpv6HopLimitTag : public Tag
{
public:
  SocketIpv6HopLimitTag ();
  void SetHopLimit (uint8_t hopLimit);
  uint8_t GetHopLimit (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_hopLimit;
};

/**
 * \brief indicated whether packets should be sent out with
 * the DF flag set.
 */
class SocketSetDontFragmentTag : public Tag
{
public:
  SocketSetDontFragmentTag ();
  void Enable (void);
  void Disable (void);
  bool IsEnabled (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;
private:
  bool m_dontFragment;
};

/*
 * \brief indicated whether the socket has IP_TOS set.
 * This tag is for IPv4 socket.
 */
class SocketIpTosTag : public Tag
{
public:
  SocketIpTosTag ();
  void SetTos (uint8_t tos);
  uint8_t GetTos (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;
private:
  uint8_t m_ipTos;
};

/*
 * \brief indicated whether the socket has IPV6_TCLASS set.
 * This tag is for IPv6 socket.
 */
class SocketIpv6TclassTag : public Tag
{
public:
  SocketIpv6TclassTag ();
  void SetTclass (uint8_t tclass);
  uint8_t GetTclass (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;
private:
  uint8_t m_ipv6Tclass;
};

/**
 * \brief This class implements a tag that carries an finishing deadline time
 * of a packet across the socket interface.
 */
class SocketDeadlineTag : public Tag
{
public:
  SocketDeadlineTag ();
  void SetDeadline (Time deadline);
  Time GetDeadline (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  Time m_deadlineFinish;
};

} // namespace ns3

#endif /* NS3_SOCKET_H */
