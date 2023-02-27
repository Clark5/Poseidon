/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "yans-error-rate-model.h"
#include "wifi-phy.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("YansErrorRateModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (YansErrorRateModel);

TypeId
YansErrorRateModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YansErrorRateModel")
    .SetParent<ErrorRateModel> ()
    .AddConstructor<YansErrorRateModel> ()
  ;
  return tid;
}

YansErrorRateModel::YansErrorRateModel ()
{
}

double
YansErrorRateModel::Log2 (double val) const
{
  return log (val) / log (2.0);
}
double
YansErrorRateModel::GetBpskBer (double snr, uint32_t signalSpread, uint32_t phyRate) const
{
  double EbNo = snr * signalSpread / phyRate;
  double z = sqrt (EbNo);
  double ber = 0.5 * erfc (z);
  NS_LOG_INFO ("bpsk snr=" << snr << " ber=" << ber);
  return ber;
}
double
YansErrorRateModel::GetQamBer (double snr, unsigned int m, uint32_t signalSpread, uint32_t phyRate) const
{
  double EbNo = snr * signalSpread / phyRate;
  double z = sqrt ((1.5 * Log2 (m) * EbNo) / (m - 1.0));
  double z1 = ((1.0 - 1.0 / sqrt (m)) * erfc (z));
  double z2 = 1 - pow ((1 - z1), 2.0);
  double ber = z2 / Log2 (m);
  NS_LOG_INFO ("Qam m=" << m << " rate=" << phyRate << " snr=" << snr << " ber=" << ber);
  return ber;
}
uint32_t
YansErrorRateModel::Factorial (uint32_t k) const
{
  uint32_t fact = 1;
  while (k > 0)
    {
      fact *= k;
      k--;
    }
  return fact;
}
double
YansErrorRateModel::Binomial (uint32_t k, double p, uint32_t n) const
{
  double retval = Factorial (n) / (Factorial (k) * Factorial (n - k)) * pow (p, k) * pow (1 - p, n - k);
  return retval;
}
double
YansErrorRateModel::CalculatePdOdd (double ber, unsigned int d) const
{
  NS_ASSERT ((d % 2) == 1);
  unsigned int dstart = (d + 1) / 2;
  unsigned int dend = d;
  double pd = 0;

  for (unsigned int i = dstart; i < dend; i++)
    {
      pd += Binomial (i, ber, d);
    }
  return pd;
}
double
YansErrorRateModel::CalculatePdEven (double ber, unsigned int d) const
{
  NS_ASSERT ((d % 2) == 0);
  unsigned int dstart = d / 2 + 1;
  unsigned int dend = d;
  double pd = 0;

  for (unsigned int i = dstart; i < dend; i++)
    {
      pd +=  Binomial (i, ber, d);
    }
  pd += 0.5 * Binomial (d / 2, ber, d);

  return pd;
}

double
YansErrorRateModel::CalculatePd (double ber, unsigned int d) const
{
  double pd;
  if ((d % 2) == 0)
    {
      pd = CalculatePdEven (ber, d);
    }
  else
    {
      pd = CalculatePdOdd (ber, d);
    }
  return pd;
}

double
YansErrorRateModel::GetFecBpskBer (double snr, double nbits,
                                   uint32_t signalSpread, uint32_t phyRate,
                                   uint32_t dFree, uint32_t adFree) const
{
  double ber = GetBpskBer (snr, signalSpread, phyRate);
  if (ber == 0.0)
    {
      return 1.0;
    }
  double pd = CalculatePd (ber, dFree);
  double pmu = adFree * pd;
  pmu = std::min (pmu, 1.0);
  double pms = pow (1 - pmu, nbits);
  return pms;
}

double
YansErrorRateModel::GetFecQamBer (double snr, uint32_t nbits,
                                  uint32_t signalSpread,
                                  uint32_t phyRate,
                                  uint32_t m, uint32_t dFree,
                                  uint32_t adFree, uint32_t adFreePlusOne) const
{
  double ber = GetQamBer (snr, m, signalSpread, phyRate);
  if (ber == 0.0)
    {
      return 1.0;
    }
  /* first term */
  double pd = CalculatePd (ber, dFree);
  double pmu = adFree * pd;
  /* second term */
  pd = CalculatePd (ber, dFree + 1);
  pmu += adFreePlusOne * pd;
  pmu = std::min (pmu, 1.0);
  double pms = pow (1 - pmu, nbits);
  return pms;
}

double
YansErrorRateModel::GetChunkSuccessRate (WifiMode mode, double snr, uint32_t nbits) const
{
  if (mode.GetModulationClass () == WIFI_MOD_CLASS_ERP_OFDM
      || mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM)
    {
      if (mode.GetConstellationSize () == 2)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_1_2)
            {
              return GetFecBpskBer (snr,
                                    nbits,
                                    mode.GetBandwidth (), // signal spread
                                    mode.GetPhyRate (), // phy rate
                                    10, // dFree
                                    11 // adFree
                                    );
            }
          else
            {
              return GetFecBpskBer (snr,
                                    nbits,
                                    mode.GetBandwidth (), // signal spread
                                    mode.GetPhyRate (), // phy rate
                                    5, // dFree
                                    8 // adFree
                                    );
            }
        }
      else if (mode.GetConstellationSize () == 4)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_1_2)
            {
              return GetFecQamBer (snr,
                                   nbits,
                                   mode.GetBandwidth (), // signal spread
                                   mode.GetPhyRate (), // phy rate
                                   4,  // m
                                   10, // dFree
                                   11, // adFree
                                   0   // adFreePlusOne
                                   );
            }
          else
            {
              return GetFecQamBer (snr,
                                   nbits,
                                   mode.GetBandwidth (), // signal spread
                                   mode.GetPhyRate (), // phy rate
                                   4, // m
                                   5, // dFree
                                   8, // adFree
                                   31 // adFreePlusOne
                                   );
            }
        }
      else if (mode.GetConstellationSize () == 16)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_1_2)
            {
              return GetFecQamBer (snr,
                                   nbits,
                                   mode.GetBandwidth (), // signal spread
                                   mode.GetPhyRate (), // phy rate
                                   16, // m
                                   10, // dFree
                                   11, // adFree
                                   0   // adFreePlusOne
                                   );
            }
          else
            {
              return GetFecQamBer (snr,
                                   nbits,
                                   mode.GetBandwidth (), // signal spread
                                   mode.GetPhyRate (), // phy rate
                                   16, // m
                                   5,  // dFree
                                   8,  // adFree
                                   31  // adFreePlusOne
                                   );
            }
        }
      else if (mode.GetConstellationSize () == 64)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_2_3)
            {
              return GetFecQamBer (snr,
                                   nbits,
                                   mode.GetBandwidth (), // signal spread
                                   mode.GetPhyRate (), // phy rate
                                   64, // m
                                   6,  // dFree
                                   1,  // adFree
                                   16  // adFreePlusOne
                                   );
            }
          else
            {
              return GetFecQamBer (snr,
                                   nbits,
                                   mode.GetBandwidth (), // signal spread
                                   mode.GetPhyRate (), // phy rate
                                   64, // m
                                   5,  // dFree
                                   8,  // adFree
                                   31  // adFreePlusOne
                                   );
            }
        }
    }
  else if (mode.GetModulationClass () == WIFI_MOD_CLASS_DSSS)
    {
      switch (mode.GetDataRate ())
        {
        case 1000000:
          return DsssErrorRateModel::GetDsssDbpskSuccessRate (snr, nbits);
        case 2000000:
          return DsssErrorRateModel::GetDsssDqpskSuccessRate (snr, nbits);
        case 5500000:
          return DsssErrorRateModel::GetDsssDqpskCck5_5SuccessRate (snr, nbits);
        case 11000000:
          return DsssErrorRateModel::GetDsssDqpskCck11SuccessRate (snr, nbits);
        }
    }
  return 0;
}

} // namespace ns3
