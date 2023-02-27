/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
#ifndef SUPPORTED_RATES_H
#define SUPPORTED_RATES_H

#include <stdint.h>
#include <ostream>
#include "ns3/buffer.h"
#include "ns3/wifi-information-element.h"

namespace ns3 {

/**
 * This defines the maximum number of supported rates that a STA is
 * allowed to have. Currently this number is set for IEEE 802.11b/g
 * stations which need 2 rates each from Clauses 15 and 18, and then 8
 * from Clause 19.
 */
#define MAX_SUPPORTED_RATES (12)

class SupportedRates;

/**
 * \brief The Extended Supported Rates Information Element
 * \ingroup wifi
 *
 * This class knows how to serialise and deserialise the Extended
 * Supported Rates Element that holds (non-HT) rates beyond the 8 that
 * the original Supported Rates element can carry.
 *
 * The \c SupportedRates class still records all the rates, and an
 * instance of \c ExtendedSupportedRatesIE lies within \c
 * SupportedRates.
 */
class ExtendedSupportedRatesIE : public WifiInformationElement
{
public:
  ExtendedSupportedRatesIE ();
  ExtendedSupportedRatesIE (SupportedRates *rates);

  WifiInformationElementId ElementId () const;
  uint8_t GetInformationFieldSize () const;
  void SerializeInformationField (Buffer::Iterator start) const;
  uint8_t DeserializeInformationField (Buffer::Iterator start,
                                       uint8_t length);

  /*
   * This information element is a bit special in that it is only
   * included if there are more than 8 rates. To support this we
   * override the Serialize and GetSerializedSize methods of
   * WifiInformationElement.
   */
  Buffer::Iterator Serialize (Buffer::Iterator start) const;
  uint16_t GetSerializedSize () const;
private:
  /**
   * This member points to the SupportedRates object that contains the
   * actual rate details. This class is a friend of that, so we have
   * access to all the private data we need.
   */
  SupportedRates *m_supportedRates;
};


/**
 * \brief The Supported Rates Information Element
 * \ingroup wifi
 *
 * This class knows how to serialise and deserialise the Supported
 * Rates Element that holds the first 8 (non-HT) supported rates.
 *
 * The \c ExtendedSupportedRatesIE class (of which an instance exists
 * in objects of this class) deals with rates beyond the first 8.
 */
class SupportedRates : public WifiInformationElement
{
public:
  SupportedRates ();

  void AddSupportedRate (uint32_t bs);
  void SetBasicRate (uint32_t bs);

  bool IsSupportedRate (uint32_t bs) const;
  bool IsBasicRate (uint32_t bs) const;

  uint8_t GetNRates (void) const;
  uint32_t GetRate (uint8_t i) const;

  WifiInformationElementId ElementId () const;
  uint8_t GetInformationFieldSize () const;
  void SerializeInformationField (Buffer::Iterator start) const;
  uint8_t DeserializeInformationField (Buffer::Iterator start,
                                       uint8_t length);

  /*
   * We support the Extended Supported Rates Information Element
   * through the ExtendedSupportedRatesIE object which is declared
   * above. We allow this class to be a friend so that it can access
   * our private data detailing the rates, and create an instance as
   * extended.
   */
  friend class ExtendedSupportedRatesIE;
  ExtendedSupportedRatesIE extended;
#ifdef WIN32
  SupportedRates& operator= (const SupportedRates& sr);
#endif
private:
  uint8_t m_nRates;
  uint8_t m_rates[MAX_SUPPORTED_RATES];
};

std::ostream &operator << (std::ostream &os, const SupportedRates &rates);

} // namespace ns3

#endif /* SUPPORTED_RATES_H */
