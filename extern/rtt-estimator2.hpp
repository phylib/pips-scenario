/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Klaus Schneider, University of Bamberg, Germany
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
 * Author: Klaus Schneider <klaus.schneider@uni-bamberg.de>
 */

#ifndef NFD_DAEMON_FW_RTT_ESTIMATOR2_HPP
#define NFD_DAEMON_FW_RTT_ESTIMATOR2_HPP

#include "common.hpp"

namespace nfd {
namespace fw {

/**
 * A simple RTT Estimator without rto calculation. Uses an exponential moving average.
 *
 * reference: ns3::RttMeanDeviation
 */
class RttEstimator2
{
public:

  /**
   * Sets the initial rtt to 10 ms and the gain to 0.1.
   *
   * @param gain The alpha/gain factor of the exponential moving average
   * @param inititalRtt defaults to 10 ms
   */
  RttEstimator2(time::microseconds initialRtt = time::milliseconds(10), double gain = 0.1);

  /**
   * Adds one new rtt measurement.
   *
   * @param measure The measured time in microseconds that should be added.
   */
  void
  addMeasurement(time::microseconds measure);

  double
  getRttInMilliseconds() const;

private:

  double rttInMicroSec;
  double gain;
  uint32_t sampleCount;
};

}  // namespace fw
}  // namespace nfd

#endif // NFD_DAEMON_FW_RTT_ESTIMATOR_NEW_HPP