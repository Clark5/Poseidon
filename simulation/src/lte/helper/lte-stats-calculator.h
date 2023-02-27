/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 */

#ifndef LTE_STATS_CALCULATOR_H_
#define LTE_STATS_CALCULATOR_H_

#include "ns3/object.h"
#include "ns3/string.h"
#include <map>

namespace ns3 {

class LteStatsCalculator : public Object
{
public:
  /**
   * Constructor
   */
  LteStatsCalculator ();

  /**
   * Destructor
   */
  virtual ~LteStatsCalculator ();

  static TypeId GetTypeId (void);

  /**
   * Set the name of the file where the uplink statistics will be stored.
   *
   * \param outputFilename string with the name of the file
   */
  void SetUlOutputFilename (std::string outputFilename);

  /**
   * Get the name of the file where the uplink statistics will be stored.
   */
  std::string GetUlOutputFilename (void);

  /**
   * Set the name of the file where the downlink statistics will be stored.
   *
   * @param outputFilename string with the name of the file
   */
  void SetDlOutputFilename (std::string outputFilename);

  /**
   * Get the name of the file where the downlink statistics will be stored.
   */
  std::string GetDlOutputFilename (void);

  /**
   * Checks if there is an already stored IMSI for the given path
   * @param path Path in the attribute system to check
   */
  bool ExistsImsiPath (std::string path);

  /**
   * Stores the (path, imsi) pairs in a map
   * @param path Path in the attribute system to store
   * @param imsi IMSI value to store
   */
  void SetImsiPath (std::string path, uint64_t imsi);

  /**
   * Retrieves the imsi information for the given path
   * @param path Path in the attribute system to get
   */
  uint64_t GetImsiPath (std::string path);

  /**
   * Checks if there is an already stored cell id for the given path
   * @param path Path in the attribute system to check
   */
  bool ExistsCellIdPath (std::string path);

  /**
   * Stores the (path, cellId) pairs in a map
   * @param path Path in the attribute system to store
   * @param cellId cell id value to store
   */
  void SetCellIdPath (std::string path, uint16_t cellId);

  /**
   * Retrieves the cell id information for the given path
   * @param path Path in the attribute system to get
   */
  uint16_t GetCellIdPath (std::string path);

private:

  std::map<std::string, uint64_t> m_pathImsiMap;
  std::map<std::string, uint16_t> m_pathCellIdMap;

  std::string m_dlOutputFilename;
  std::string m_ulOutputFilename;
};

} // namespace ns3

#endif /* LTE_STATS_CALCULATOR_H_ */
