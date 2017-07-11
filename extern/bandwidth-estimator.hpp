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

#ifndef NFD_DAEMON_FW_BANDWIDTH_ESTIMATOR_HPP
#define NFD_DAEMON_FW_BANDWIDTH_ESTIMATOR_HPP

#include "common.hpp"
#include <boost/chrono/duration.hpp>
#include <ndn-cxx/util/time.hpp>
#include <map>

namespace nfd {
namespace fw {

/**
 * Implements a bandwidth estimator using a simple moving average.
 * The average bandwidth is calculated over a sliding window of x time units.
 */
class BandwidthEstimator
{
public:

  /**
   * Constructs the bandwidth estimator and sets the sliding window size
   *
   * @param window the sliding window size
   */
  BandwidthEstimator(time::steady_clock::duration window);

  /**
   * Adds one interest or data packet to the bandwidth calculation.
   * 
   * @param sizeInBytes The size of the packet in bytes.
   */
  void
  addPacket(size_t sizeInBytes);

  /**
   * @returns the bandwidth over the sliding window in kilobytes per second (1 KB = 1024 bytes).
   * @returns 0 if there were no data packets inside the time window.
   */
  double
  getKBytesPerSecond();

private:

  // The sliding window size
  const time::steady_clock::duration windowSize;

  // A map of time stamps and corresponding size of packets
  std::map<time::steady_clock::TimePoint, size_t> bwMap;

};

}  // namespace fw
}  // namespace nfd

#endif // NFD_DAEMON_FW_BANDWIDTH_ESTIMATOR_HPP