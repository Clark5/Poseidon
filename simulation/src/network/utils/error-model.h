/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 *
 * This file incorporates work covered by the following copyright and  
 * permission notice:   
 *
 * Copyright (c) 1997 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Contributed by the Daedalus Research Group, UC Berkeley
 * (http://daedalus.cs.berkeley.edu)
 *
 * This code has been ported from ns-2 (queue/errmodel.{cc,h}
 */

#ifndef ERROR_MODEL_H
#define ERROR_MODEL_H

#include <list>
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class Packet;

/**
 * \ingroup network
 * \defgroup errormodel Error Model
 */
/**
 * \ingroup errormodel
 * \brief General error model that can be used to corrupt packets
 *
 * This object is used to flag packets as being lost/errored or not.
 * It is part of the Object framework and can be aggregated to 
 * other ns3 objects and handled by the Ptr class.
 *
 * The main method is IsCorrupt(Ptr<Packet> p) which returns true if 
 * the packet is to be corrupted according to the underlying model.
 * Depending on the error model, the packet itself may have its packet
 * data buffer errored or not, or side information may be returned to
 * the client in the form of a packet tag.  (Note:  No such error models
 * that actually error the bits in a packet presently exist).
 * The object can have state (resettable by Reset()).
 * The object can also be enabled and disabled via two public member functions.
 * 
 * Typical code (simplified) to use an ErrorModel may look something like 
 * this:
 * \code 
 * Ptr<ErrorModel> rem = CreateObject<RateErrorModel> ();
 * Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
 * rem->SetRandomVariable (uv);
 * rem->SetRate (0.001);
 * ...
 * Ptr<Packet> p;
 * if (rem->IsCorrupt (p))
 *   {
 *     dropTrace(p);
 *   } else {
 *     Forward (p);
 *   }
 * \endcode
 *
 * Two practical error models, a ListErrorModel and a RateErrorModel,
 * are currently implemented. 
 */
class ErrorModel : public Object
{
public:
  static TypeId GetTypeId (void);

  ErrorModel ();
  virtual ~ErrorModel ();

  /**
   * Note:  Depending on the error model, this function may or may not
   * alter the contents of the packet upon returning true.
   *
   * \returns true if the Packet is to be considered as errored/corrupted
   * \param pkt Packet to apply error model to
   */
  bool IsCorrupt (Ptr<Packet> pkt);
  /**
   * Reset any state associated with the error model
   */
  void Reset (void);
  /**
   * Enable the error model
   */
  void Enable (void);
  /**
   * Disable the error model
   */
  void Disable (void);
  /**
   * \return true if error model is enabled; false otherwise
   */
  bool IsEnabled (void) const;

private:
  /*
   * These methods must be implemented by subclasses
   */
  virtual bool DoCorrupt (Ptr<Packet>) = 0;
  virtual void DoReset (void) = 0;

  bool m_enable;
};

/**
 * \brief Determine which packets are errored corresponding to an underlying
 * distribution, rate, and unit.
 *
 * This object is used to flag packets as being lost/errored or not.
 * The two parameters that govern the behavior are the rate (or
 * equivalently, the mean duration/spacing between errors), and the
 * unit (which may be per-bit, per-byte, and per-packet).
 * Users can optionally provide a RandomVariableStream object; the default
 * is to use a Uniform(0,1) distribution.

 * Reset() on this model will do nothing
 *
 * IsCorrupt() will not modify the packet data buffer
 */
class RateErrorModel : public ErrorModel
{
public:
  static TypeId GetTypeId (void);

  RateErrorModel ();
  virtual ~RateErrorModel ();

  enum ErrorUnit
  {
    ERROR_UNIT_BIT,
    ERROR_UNIT_BYTE,
    ERROR_UNIT_PACKET
  };

  /**
   * \returns the ErrorUnit being used by the underlying model
   */ 
  RateErrorModel::ErrorUnit GetUnit (void) const;
  /**
   * \param error_unit the ErrorUnit to be used by the underlying model
   */ 
  void SetUnit (enum ErrorUnit error_unit);

  /**
   * \returns the error rate being applied by the model
   */ 
  double GetRate (void) const;
  /**
   * \param rate the error rate to be used by the model
   */ 
  void SetRate (double rate);

  /**
   * \param ranvar A random variable distribution to generate random variates
   */ 
  void SetRandomVariable (Ptr<RandomVariableStream>);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

private:
  virtual bool DoCorrupt (Ptr<Packet> p);
  virtual bool DoCorruptPkt (Ptr<Packet> p);
  virtual bool DoCorruptByte (Ptr<Packet> p);
  virtual bool DoCorruptBit (Ptr<Packet> p);
  virtual void DoReset (void);

  enum ErrorUnit m_unit;
  double m_rate;

  Ptr<RandomVariableStream> m_ranvar;
};

/**
 * \brief Provide a list of Packet uids to corrupt
 *
 * This object is used to flag packets as being lost/errored or not.
 * A note on performance:  the list is assumed to be unordered, and
 * in general, Packet uids received may be unordered.  Therefore,
 * each call to IsCorrupt() will result in a walk of the list with
 * the present underlying implementation.
 * 
 * Note also that if one wants to target multiple packets from looking
 * at an (unerrored) trace file, the act of erroring a given packet may
 * cause subsequent packet uids to change.  For instance, suppose one wants 
 * to error packets 11 and 17 on a given device.  It may be that erroring
 * packet 11 will cause the subsequent uid stream to change and 17 may no
 * longer correspond to the second packet that one wants to lose.  Therefore,
 * be advised that it might take some trial and error to select the
 * right uids when multiple are provided.
 * 
 * Reset() on this model will clear the list
 *
 * IsCorrupt() will not modify the packet data buffer
 */
class ListErrorModel : public ErrorModel
{
public:
  static TypeId GetTypeId (void);
  ListErrorModel ();
  virtual ~ListErrorModel ();

  /**
   * \return a copy of the underlying list
   */
  std::list<uint32_t> GetList (void) const;
  /**
   * \param packetlist The list of packet uids to error.
   *
   * This method overwrites any previously provided list.
   */
  void SetList (const std::list<uint32_t> &packetlist);

private:
  virtual bool DoCorrupt (Ptr<Packet> p);
  virtual void DoReset (void);

  typedef std::list<uint32_t> PacketList;
  typedef std::list<uint32_t>::const_iterator PacketListCI;

  PacketList m_packetList;

};

/**
 * \brief Provide a list of Packets to corrupt
 *
 * This model also processes a user-generated list of packets to
 * corrupt, except that the list corresponds to the sequence of
 * received packets as observed by this error model, and not the
 * Packet UID.
 * 
 * Reset() on this model will clear the list
 *
 * IsCorrupt() will not modify the packet data buffer
 */
class ReceiveListErrorModel : public ErrorModel
{
public:
  static TypeId GetTypeId (void);
  ReceiveListErrorModel ();
  virtual ~ReceiveListErrorModel ();

  /**
   * \return a copy of the underlying list
   */
  std::list<uint32_t> GetList (void) const;
  /**
   * \param packetlist The list of packets to error.
   *
   * This method overwrites any previously provided list.
   */
  void SetList (const std::list<uint32_t> &packetlist);

private:
  virtual bool DoCorrupt (Ptr<Packet> p);
  virtual void DoReset (void);

  typedef std::list<uint32_t> PacketList;
  typedef std::list<uint32_t>::const_iterator PacketListCI;

  PacketList m_packetList;
  uint32_t m_timesInvoked;

};


} // namespace ns3
#endif
