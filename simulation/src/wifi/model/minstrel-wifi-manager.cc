/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Duy Nguyen
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
 * Author: Duy Nguyen <duy@soe.ucsc.edu>
 *
 * Some Comments:
 *
 * 1) Segment Size is declared for completeness but not used  because it has
 *    to do more with the requirement of the specific hardware.
 *
 * 2) By default, Minstrel applies the multi-rate retry(the core of Minstrel
 *    algorithm). Otherwise, please use ConstantRateWifiManager instead.
 *
 * http://linuxwireless.org/en/developers/Documentation/mac80211/RateControl/minstrel
 */

#include "minstrel-wifi-manager.h"
#include "wifi-phy.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/wifi-mac.h"
#include "ns3/assert.h"
#include <vector>

NS_LOG_COMPONENT_DEFINE ("MinstrelWifiManager");


namespace ns3 {


struct MinstrelWifiRemoteStation : public WifiRemoteStation
{
  Time m_nextStatsUpdate;  ///< 10 times every second

  /**
   * To keep track of the current position in the our random sample table
   * going row by row from 1st column until the 10th column(Minstrel defines 10)
   * then we wrap back to the row 1 col 1.
   * note: there are many other ways to do this.
   */
  uint32_t m_col, m_index;
  uint32_t m_maxTpRate;  ///< the current throughput rate
  uint32_t m_maxTpRate2;  ///< second highest throughput rate
  uint32_t m_maxProbRate;  ///< rate with highest prob of success

  int m_packetCount;  ///< total number of packets as of now
  int m_sampleCount;  ///< how many packets we have sample so far

  bool m_isSampling;  ///< a flag to indicate we are currently sampling
  uint32_t m_sampleRate;  ///< current sample rate
  bool  m_sampleRateSlower;  ///< a flag to indicate sample rate is slower
  uint32_t m_currentRate;  ///< current rate we are using

  uint32_t m_shortRetry;  ///< short retries such as control packts
  uint32_t m_longRetry;  ///< long retries such as data packets
  uint32_t m_retry;  ///< total retries short + long
  uint32_t m_err;  ///< retry errors
  uint32_t m_txrate;  ///< current transmit rate

  bool m_initialized;  ///< for initializing tables
};

NS_OBJECT_ENSURE_REGISTERED (MinstrelWifiManager);

TypeId
MinstrelWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MinstrelWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .AddConstructor<MinstrelWifiManager> ()
    .AddAttribute ("UpdateStatistics",
                   "The interval between updating statistics table ",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&MinstrelWifiManager::m_updateStats),
                   MakeTimeChecker ())
    .AddAttribute ("LookAroundRate",
                   "the percentage to try other rates",
                   DoubleValue (10),
                   MakeDoubleAccessor (&MinstrelWifiManager::m_lookAroundRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("EWMA",
                   "EWMA level",
                   DoubleValue (75),
                   MakeDoubleAccessor (&MinstrelWifiManager::m_ewmaLevel),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SegmentSize",
                   "The largest allowable segment size packet",
                   DoubleValue (6000),
                   MakeDoubleAccessor (&MinstrelWifiManager::m_segmentSize),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("SampleColumn",
                   "The number of columns used for sampling",
                   DoubleValue (10),
                   MakeDoubleAccessor (&MinstrelWifiManager::m_sampleCol),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("PacketLength",
                   "The packet length used for calculating mode TxTime",
                   DoubleValue (1200),
                   MakeDoubleAccessor (&MinstrelWifiManager::m_pktLen),
                   MakeDoubleChecker <double> ())
  ;
  return tid;
}

MinstrelWifiManager::MinstrelWifiManager ()
{
  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();

  m_nsupported = 0;
}

MinstrelWifiManager::~MinstrelWifiManager ()
{
}

void
MinstrelWifiManager::SetupPhy (Ptr<WifiPhy> phy)
{
  uint32_t nModes = phy->GetNModes ();
  for (uint32_t i = 0; i < nModes; i++)
    {
      WifiMode mode = phy->GetMode (i);
      AddCalcTxTime (mode, phy->CalculateTxDuration (m_pktLen, mode, WIFI_PREAMBLE_LONG));
    }
  WifiRemoteStationManager::SetupPhy (phy);
}

int64_t
MinstrelWifiManager::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}

Time
MinstrelWifiManager::GetCalcTxTime (WifiMode mode) const
{

  for (TxTime::const_iterator i = m_calcTxTime.begin (); i != m_calcTxTime.end (); i++)
    {
      if (mode == i->second)
        {
          return i->first;
        }
    }
  NS_ASSERT (false);
  return Seconds (0);
}

void
MinstrelWifiManager::AddCalcTxTime (WifiMode mode, Time t)
{
  m_calcTxTime.push_back (std::make_pair (t, mode));
}

WifiRemoteStation *
MinstrelWifiManager::DoCreateStation (void) const
{
  MinstrelWifiRemoteStation *station = new MinstrelWifiRemoteStation ();

  station->m_nextStatsUpdate = Simulator::Now () + m_updateStats;
  station->m_col = 0;
  station->m_index = 0;
  station->m_maxTpRate = 0;
  station->m_maxTpRate2 = 0;
  station->m_maxProbRate = 0;
  station->m_packetCount = 0;
  station->m_sampleCount = 0;
  station->m_isSampling = false;
  station->m_sampleRate = 0;
  station->m_sampleRateSlower = false;
  station->m_currentRate = 0;
  station->m_shortRetry = 0;
  station->m_longRetry = 0;
  station->m_retry = 0;
  station->m_err = 0;
  station->m_txrate = 0;
  station->m_initialized = false;

  return station;
}

void
MinstrelWifiManager::CheckInit (MinstrelWifiRemoteStation *station)
{
  if (!station->m_initialized && GetNSupported (station) > 1)
    {
      // Note: we appear to be doing late initialization of the table
      // to make sure that the set of supported rates has been initialized
      // before we perform our own initialization.
      m_nsupported = GetNSupported (station);
      m_minstrelTable = MinstrelRate (m_nsupported);
      m_sampleTable = SampleRate (m_nsupported, std::vector<uint32_t> (m_sampleCol));
      InitSampleTable (station);
      RateInit (station);
      station->m_initialized = true;
    }
}

void
MinstrelWifiManager::DoReportRxOk (WifiRemoteStation *st,
                                   double rxSnr, WifiMode txMode)
{
  NS_LOG_DEBUG ("DoReportRxOk m_txrate=" << ((MinstrelWifiRemoteStation *)st)->m_txrate);
}

void
MinstrelWifiManager::DoReportRtsFailed (WifiRemoteStation *st)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *)st;
  NS_LOG_DEBUG ("DoReportRtsFailed m_txrate=" << station->m_txrate);

  station->m_shortRetry++;
}

void
MinstrelWifiManager::DoReportRtsOk (WifiRemoteStation *st, double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_DEBUG ("self=" << st << " rts ok");
}

void
MinstrelWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *st)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *)st;
  UpdateRetry (station);
  station->m_err++;
}

void
MinstrelWifiManager::DoReportDataFailed (WifiRemoteStation *st)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *)st;
  /**
   *
   * Retry Chain table is implemented here
   *
   * Try |         LOOKAROUND RATE              | NORMAL RATE
   *     | random < best    | random > best     |
   * --------------------------------------------------------------
   *  1  | Best throughput  | Random rate       | Best throughput
   *  2  | Random rate      | Best throughput   | Next best throughput
   *  3  | Best probability | Best probability  | Best probability
   *  4  | Lowest Baserate  | Lowest baserate   | Lowest baserate
   *
   * Note: For clarity, multiple blocks of if's and else's are used
   * After a failing 7 times, DoReportFinalDataFailed will be called
   */

  CheckInit (station);
  if (!station->m_initialized)
    {
      return;
    }

  station->m_longRetry++;

  NS_LOG_DEBUG ("DoReportDataFailed " << station << "\t rate " << station->m_txrate << "\tlongRetry \t" << station->m_longRetry);

  /// for normal rate, we're not currently sampling random rates
  if (!station->m_isSampling)
    {
      /// use best throughput rate
      if (station->m_longRetry < m_minstrelTable[station->m_txrate].adjustedRetryCount)
        {
          ;  ///<  there's still a few retries left
        }

      /// use second best throughput rate
      else if (station->m_longRetry <= (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                        m_minstrelTable[station->m_maxTpRate].adjustedRetryCount))
        {
          station->m_txrate = station->m_maxTpRate2;
        }

      /// use best probability rate
      else if (station->m_longRetry <= (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                        m_minstrelTable[station->m_maxTpRate2].adjustedRetryCount +
                                        m_minstrelTable[station->m_maxTpRate].adjustedRetryCount))
        {
          station->m_txrate = station->m_maxProbRate;
        }

      /// use lowest base rate
      else if (station->m_longRetry > (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                       m_minstrelTable[station->m_maxTpRate2].adjustedRetryCount +
                                       m_minstrelTable[station->m_maxTpRate].adjustedRetryCount))
        {
          station->m_txrate = 0;
        }
    }

  /// for look-around rate, we're currently sampling random rates
  else
    {
      /// current sampling rate is slower than the current best rate
      if (station->m_sampleRateSlower)
        {
          /// use best throughput rate
          if (station->m_longRetry < m_minstrelTable[station->m_txrate].adjustedRetryCount)
            {
              ; ///<  there are a few retries left
            }

          ///	use random rate
          else if (station->m_longRetry <= (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                            m_minstrelTable[station->m_maxTpRate].adjustedRetryCount))
            {
              station->m_txrate = station->m_sampleRate;
            }

          /// use max probability rate
          else if (station->m_longRetry <= (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                            m_minstrelTable[station->m_sampleRate].adjustedRetryCount +
                                            m_minstrelTable[station->m_maxTpRate].adjustedRetryCount ))
            {
              station->m_txrate = station->m_maxProbRate;
            }

          /// use lowest base rate
          else if (station->m_longRetry > (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                           m_minstrelTable[station->m_sampleRate].adjustedRetryCount +
                                           m_minstrelTable[station->m_maxTpRate].adjustedRetryCount))
            {
              station->m_txrate = 0;
            }
        }

      /// current sampling rate is better than current best rate
      else
        {
          /// use random rate
          if (station->m_longRetry < m_minstrelTable[station->m_txrate].adjustedRetryCount)
            {
              ;    ///< keep using it
            }

          /// use the best rate
          else if (station->m_longRetry <= (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                            m_minstrelTable[station->m_sampleRate].adjustedRetryCount))
            {
              station->m_txrate = station->m_maxTpRate;
            }

          /// use the best probability rate
          else if (station->m_longRetry <= (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                            m_minstrelTable[station->m_maxTpRate].adjustedRetryCount +
                                            m_minstrelTable[station->m_sampleRate].adjustedRetryCount))
            {
              station->m_txrate = station->m_maxProbRate;
            }

          /// use the lowest base rate
          else if (station->m_longRetry > (m_minstrelTable[station->m_txrate].adjustedRetryCount +
                                           m_minstrelTable[station->m_maxTpRate].adjustedRetryCount +
                                           m_minstrelTable[station->m_sampleRate].adjustedRetryCount))
            {
              station->m_txrate = 0;
            }
        }
    }
}

void
MinstrelWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                     double ackSnr, WifiMode ackMode, double dataSnr)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *) st;

  station->m_isSampling = false;
  station->m_sampleRateSlower = false;

  CheckInit (station);
  if (!station->m_initialized)
    {
      return;
    }

  m_minstrelTable[station->m_txrate].numRateSuccess++;
  m_minstrelTable[station->m_txrate].numRateAttempt++;

  UpdateRetry (station);

  m_minstrelTable[station->m_txrate].numRateAttempt += station->m_retry;
  station->m_packetCount++;

  if (m_nsupported >= 1)
    {
      station->m_txrate = FindRate (station);
    }
}

void
MinstrelWifiManager::DoReportFinalDataFailed (WifiRemoteStation *st)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *) st;
  NS_LOG_DEBUG ("DoReportFinalDataFailed m_txrate=" << station->m_txrate);

  station->m_isSampling = false;
  station->m_sampleRateSlower = false;

  UpdateRetry (station);

  m_minstrelTable[station->m_txrate].numRateAttempt += station->m_retry;
  station->m_err++;

  if (m_nsupported >= 1)
    {
      station->m_txrate = FindRate (station);
    }
}

void
MinstrelWifiManager::UpdateRetry (MinstrelWifiRemoteStation *station)
{
  station->m_retry = station->m_shortRetry + station->m_longRetry;
  station->m_shortRetry = 0;
  station->m_longRetry = 0;
}

WifiMode
MinstrelWifiManager::DoGetDataMode (WifiRemoteStation *st,
                                    uint32_t size)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *) st;
  if (!station->m_initialized)
    {
      CheckInit (station);

      /// start the rate at half way
      station->m_txrate = m_nsupported / 2;
    }
  UpdateStats (station);
  return GetSupported (station, station->m_txrate);
}

WifiMode
MinstrelWifiManager::DoGetRtsMode (WifiRemoteStation *st)
{
  MinstrelWifiRemoteStation *station = (MinstrelWifiRemoteStation *) st;
  NS_LOG_DEBUG ("DoGetRtsMode m_txrate=" << station->m_txrate);

  return GetSupported (station, 0);
}

bool
MinstrelWifiManager::IsLowLatency (void) const
{
  return true;
}
uint32_t
MinstrelWifiManager::GetNextSample (MinstrelWifiRemoteStation *station)
{
  uint32_t bitrate;
  bitrate = m_sampleTable[station->m_index][station->m_col];
  station->m_index++;

  /// bookeeping for m_index and m_col variables
  if (station->m_index > (m_nsupported - 2))
    {
      station->m_index = 0;
      station->m_col++;
      if (station->m_col >= m_sampleCol)
        {
          station->m_col = 0;
        }
    }
  return bitrate;
}

uint32_t
MinstrelWifiManager::FindRate (MinstrelWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("FindRate " << "packet=" << station->m_packetCount );

  if ((station->m_sampleCount + station->m_packetCount) == 0)
    {
      return 0;
    }


  uint32_t idx;

  /// for determining when to try a sample rate
  int coinFlip = m_uniformRandomVariable->GetInteger (0, 100) % 2;

  /**
   * if we are below the target of look around rate percentage, look around
   * note: do it randomly by flipping a coin instead sampling
   * all at once until it reaches the look around rate
   */
  if ( (((100 * station->m_sampleCount) / (station->m_sampleCount + station->m_packetCount )) < m_lookAroundRate)
       && (coinFlip == 1) )
    {

      /// now go through the table and find an index rate
      idx = GetNextSample (station);


      /**
       * This if condition is used to make sure that we don't need to use
       * the sample rate it is the same as our current rate
       */
      if (idx != station->m_maxTpRate && idx != station->m_txrate)
        {

          /// start sample count
          station->m_sampleCount++;

          /// set flag that we are currently sampling
          station->m_isSampling = true;

          /// bookeeping for resetting stuff
          if (station->m_packetCount >= 10000)
            {
              station->m_sampleCount = 0;
              station->m_packetCount = 0;
            }

          /// error check
          if (idx >= m_nsupported)
            {
              NS_LOG_DEBUG ("ALERT!!! ERROR");
            }

          /// set the rate that we're currently sampling
          station->m_sampleRate = idx;

          if (station->m_sampleRate == station->m_maxTpRate)
            {
              station->m_sampleRate = station->m_maxTpRate2;
            }

          /// is this rate slower than the current best rate
          station->m_sampleRateSlower =
            (m_minstrelTable[idx].perfectTxTime > m_minstrelTable[station->m_maxTpRate].perfectTxTime);

          /// using the best rate instead
          if (station->m_sampleRateSlower)
            {
              idx =  station->m_maxTpRate;
            }
        }

    }

  ///	continue using the best rate
  else
    {
      idx = station->m_maxTpRate;
    }


  NS_LOG_DEBUG ("FindRate " << "sample rate=" << idx);

  return idx;
}

void
MinstrelWifiManager::UpdateStats (MinstrelWifiRemoteStation *station)
{
  if (Simulator::Now () <  station->m_nextStatsUpdate)
    {
      return;
    }

  if (!station->m_initialized)
    {
      return;
    }
  NS_LOG_DEBUG ("Updating stats=" << this);

  station->m_nextStatsUpdate = Simulator::Now () + m_updateStats;

  Time txTime;
  uint32_t tempProb;

  for (uint32_t i = 0; i < m_nsupported; i++)
    {

      /// calculate the perfect tx time for this rate
      txTime = m_minstrelTable[i].perfectTxTime;

      /// just for initialization
      if (txTime.GetMicroSeconds () == 0)
        {
          txTime = Seconds (1);
        }

      NS_LOG_DEBUG ("m_txrate=" << station->m_txrate <<
                    "\t attempt=" << m_minstrelTable[i].numRateAttempt <<
                    "\t success=" << m_minstrelTable[i].numRateSuccess);

      /// if we've attempted something
      if (m_minstrelTable[i].numRateAttempt)
        {
          /**
           * calculate the probability of success
           * assume probability scales from 0 to 18000
           */
          tempProb = (m_minstrelTable[i].numRateSuccess * 18000) / m_minstrelTable[i].numRateAttempt;

          /// bookeeping
          m_minstrelTable[i].successHist += m_minstrelTable[i].numRateSuccess;
          m_minstrelTable[i].attemptHist += m_minstrelTable[i].numRateAttempt;
          m_minstrelTable[i].prob = tempProb;

          /// ewma probability (cast for gcc 3.4 compatibility)
          tempProb = static_cast<uint32_t> (((tempProb * (100 - m_ewmaLevel)) + (m_minstrelTable[i].ewmaProb * m_ewmaLevel) ) / 100);

          m_minstrelTable[i].ewmaProb = tempProb;

          /// calculating throughput
          m_minstrelTable[i].throughput = tempProb * (1000000 / txTime.GetMicroSeconds ());

        }

      /// bookeeping
      m_minstrelTable[i].prevNumRateAttempt = m_minstrelTable[i].numRateAttempt;
      m_minstrelTable[i].prevNumRateSuccess = m_minstrelTable[i].numRateSuccess;
      m_minstrelTable[i].numRateSuccess = 0;
      m_minstrelTable[i].numRateAttempt = 0;

      /// Sample less often below 10% and  above 95% of success
      if ((m_minstrelTable[i].ewmaProb > 17100) || (m_minstrelTable[i].ewmaProb < 1800))
        {
          /**
           * retry count denotes the number of retries permitted for each rate
           * # retry_count/2
           */
          m_minstrelTable[i].adjustedRetryCount = m_minstrelTable[i].retryCount >> 1;
          if (m_minstrelTable[i].adjustedRetryCount > 2)
            {
              m_minstrelTable[i].adjustedRetryCount = 2;
            }
        }
      else
        {
          m_minstrelTable[i].adjustedRetryCount = m_minstrelTable[i].retryCount;
        }

      /// if it's 0 allow one retry limit
      if (m_minstrelTable[i].adjustedRetryCount == 0)
        {
          m_minstrelTable[i].adjustedRetryCount = 1;
        }
    }


  uint32_t max_prob = 0, index_max_prob = 0, max_tp = 0, index_max_tp = 0, index_max_tp2 = 0;

  /// go find max throughput, second maximum throughput, high probability succ
  for (uint32_t i = 0; i < m_nsupported; i++)
    {
      NS_LOG_DEBUG ("throughput" << m_minstrelTable[i].throughput <<
                    "\n ewma" << m_minstrelTable[i].ewmaProb);

      if (max_tp < m_minstrelTable[i].throughput)
        {
          index_max_tp = i;
          max_tp = m_minstrelTable[i].throughput;
        }

      if (max_prob < m_minstrelTable[i].ewmaProb)
        {
          index_max_prob = i;
          max_prob = m_minstrelTable[i].ewmaProb;
        }
    }


  max_tp = 0;
  /// find the second highest max
  for (uint32_t i = 0; i < m_nsupported; i++)
    {
      if ((i != index_max_tp) && (max_tp < m_minstrelTable[i].throughput))
        {
          index_max_tp2 = i;
          max_tp = m_minstrelTable[i].throughput;
        }
    }

  station->m_maxTpRate = index_max_tp;
  station->m_maxTpRate2 = index_max_tp2;
  station->m_maxProbRate = index_max_prob;
  station->m_currentRate = index_max_tp;

  if (index_max_tp > station->m_txrate)
    {
      station->m_txrate = index_max_tp;
    }

  NS_LOG_DEBUG ("max tp=" << index_max_tp << "\nmax tp2=" << index_max_tp2 << "\nmax prob=" << index_max_prob);

  /// reset it
  RateInit (station);
}

void
MinstrelWifiManager::RateInit (MinstrelWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("RateInit=" << station);

  for (uint32_t i = 0; i < m_nsupported; i++)
    {
      m_minstrelTable[i].numRateAttempt = 0;
      m_minstrelTable[i].numRateSuccess = 0;
      m_minstrelTable[i].prob = 0;
      m_minstrelTable[i].ewmaProb = 0;
      m_minstrelTable[i].prevNumRateAttempt = 0;
      m_minstrelTable[i].prevNumRateSuccess = 0;
      m_minstrelTable[i].successHist = 0;
      m_minstrelTable[i].attemptHist = 0;
      m_minstrelTable[i].throughput = 0;
      m_minstrelTable[i].perfectTxTime = GetCalcTxTime (GetSupported (station, i));
      m_minstrelTable[i].retryCount = 1;
      m_minstrelTable[i].adjustedRetryCount = 1;
    }
}

void
MinstrelWifiManager::InitSampleTable (MinstrelWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("InitSampleTable=" << this);

  station->m_col = station->m_index = 0;

  /// for off-seting to make rates fall between 0 and numrates
  uint32_t numSampleRates = m_nsupported;

  uint32_t newIndex;
  for (uint32_t col = 0; col < m_sampleCol; col++)
    {
      for (uint32_t i = 0; i < numSampleRates; i++ )
        {

          /**
           * The next two lines basically tries to generate a random number
           * between 0 and the number of available rates
           */
          int uv = m_uniformRandomVariable->GetInteger (0, numSampleRates);
          newIndex = (i + uv) % numSampleRates;

          /// this loop is used for filling in other uninitilized places
          while (m_sampleTable[newIndex][col] != 0)
            {
              newIndex = (newIndex + 1) % m_nsupported;
            }
          m_sampleTable[newIndex][col] = i;

        }
    }
}

void
MinstrelWifiManager::PrintSampleTable (MinstrelWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("PrintSampleTable=" << station);

  uint32_t numSampleRates = m_nsupported;
  for (uint32_t i = 0; i < numSampleRates; i++)
    {
      for (uint32_t j = 0; j < m_sampleCol; j++)
        {
          std::cout << m_sampleTable[i][j] << "\t";
        }
      std::cout << std::endl;
    }
}

void
MinstrelWifiManager::PrintTable (MinstrelWifiRemoteStation *station)
{
  NS_LOG_DEBUG ("PrintTable=" << station);

  for (uint32_t i = 0; i < m_nsupported; i++)
    {
      std::cout << "index(" << i << ") = " << m_minstrelTable[i].perfectTxTime << "\n";
    }
}

} // namespace ns3





