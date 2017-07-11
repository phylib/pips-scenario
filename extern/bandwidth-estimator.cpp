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

#include "bandwidth-estimator.hpp"

namespace nfd {
namespace fw {

BandwidthEstimator::BandwidthEstimator(time::steady_clock::duration window) :
    windowSize(window)
{
}

void BandwidthEstimator::addPacket(size_t sizeInBytes)
{
  time::steady_clock::TimePoint now = time::steady_clock::now();
  bwMap[now] = sizeInBytes;
}

double BandwidthEstimator::getKBytesPerSecond()
{
  time::steady_clock::time_point now = time::steady_clock::now();

  // Remove too early data packets
  time::steady_clock::TimePoint lastValidInterests = now - windowSize;
  bwMap.erase(bwMap.begin(), bwMap.upper_bound(lastValidInterests));

  // Return 0 if no packets are inside the sliding window
  if (bwMap.size() == 0) {
    return 0;
  }
  // Else return the data rate
  else {
    size_t totalSize = 0;
    for (auto n : bwMap) {
      totalSize += n.second;
    }
    double windowSeconds = windowSize.count() / 1000000000.0;
    double kiloBytesPerSec = (double) totalSize / ((double) windowSeconds * 1024);

    return kiloBytesPerSec;
  }
}

}  // namespace fw
}  // namespace nfd