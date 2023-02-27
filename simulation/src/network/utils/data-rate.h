/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: Rajib Bhattacharjea<raj.b@gatech.edu>
//

#ifndef DATA_RATE_H
#define DATA_RATE_H

#include <string>
#include <iostream>
#include <stdint.h>
#include "ns3/nstime.h"
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"

namespace ns3 {

/**
 * \ingroup network
 * \defgroup datarate Data Rate
 */
/**
 * \ingroup datarate
 * \brief Class for representing data rates
 *
 * Allows for natural and familiar use of data rates.  Allows construction
 * from strings, natural multiplication e.g.:
 * \code
 * DataRate x("56kbps");
 * double nBits = x*ns3::Seconds(19.2);
 * uint32_t nBytes = 20;
 * double txtime = x.CalclulateTxTime(nBytes);
 * \endcode
 * This class also supports the regular comparison operators <, >, <=, >=, ==,
 * and !=
 *
 * Conventions used:
 * "b" stands for bits, "B" for bytes (8 bits) \n
 * "k" stands for 1000, "K" also stands for 1000, "Ki" stands for 1024 \n
 * "M" stand for 1000000, "Mib" stands for 1024 kibibits, or 1048576 bits \n
 * "G" stand for 10^9, "Gib" stands for 1024 mebibits \n
 * whitespace is allowed but not required between the numeric value and units
 *
 * Supported unit strings:
 * bps, b/s, Bps, B/s \n
 * kbps, kb/s, Kbps, Kb/s, kBps, kB/s, KBps, KB/s, Kib/s, KiB/s \n
 * Mbps, Mb/s, MBps, MB/s, Mib/s, MiB/s \n
 * Gbps, Gb/s, GBps, GB/s, Gib/s, GiB/s \n
 * 
 * Examples:
 * "56kbps" = 56,000 bits/s \n
 * "128 kb/s" = 128,000 bits/s \n
 * "8Kib/s" = 1 KiB/s = 8192 bits/s \n
 * "1kB/s" = 8000 bits/s 
 */
class DataRate
{
public:
  DataRate ();
  /**
   * \brief Integer constructor
   *
   * Construct a data rate from an integer.  This class only supports positive
   * integer data rates in units of bits/s, meaning 1bit/s is the smallest 
   * non-trivial bitrate available.
   * \param bps bit/s value
   */
  DataRate (uint64_t bps);
  DataRate (std::string rate);

  bool operator <  (const DataRate& rhs) const;
  bool operator <= (const DataRate& rhs) const;
  bool operator >  (const DataRate& rhs) const;
  bool operator >= (const DataRate& rhs) const;
  bool operator == (const DataRate& rhs) const;
  bool operator != (const DataRate& rhs) const;

  DataRate& operator /=(const double& c);
  DataRate& operator +=(const DataRate& r);

  /**
   * \brief Calculate transmission time
   *
   * Calculates the transmission time at this data rate
   * \param bytes The number of bytes (not bits) for which to calculate
   * \return The transmission time in seconds for the number of bytes specified
   */
  double CalculateTxTime (uint32_t bytes) const;

  /**
   * Get the underlying bitrate
   * \return The underlying bitrate in bits per second
   */
  uint64_t GetBitRate () const;

private:
  uint64_t m_bps;
  static uint64_t Parse (const std::string);
};

std::ostream &operator << (std::ostream &os, const DataRate &rate);
std::istream &operator >> (std::istream &is, DataRate &rate);

/**
 * \class ns3::DataRateValue
 * \brief hold objects of type ns3::DataRate
 */

ATTRIBUTE_HELPER_HEADER (DataRate);

/**
 * \param lhs
 * \param rhs
 * \return Bits transmitted in rhs seconds at lhs b/s
 */
double operator* (const DataRate& lhs, const Time& rhs);
double operator* (const Time& lhs, const DataRate& rhs);

DataRate operator*(const double& c, const DataRate& d);
DataRate operator*(const DataRate& d, const double& c);

DataRate operator/(const DataRate& d, const double& c);
double operator/(const DataRate& lhs, const DataRate& rhs);

DataRate operator+(const DataRate& lhs, const DataRate& rhs);

} // namespace ns3

#endif /* DATA_RATE_H */
