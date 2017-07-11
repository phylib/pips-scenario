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

#include "rtt-estimator2.hpp"

namespace nfd {
namespace fw {

RttEstimator2::RttEstimator2(time::microseconds initialRtt, double gain) :
    rttInMicroSec(initialRtt.count()), gain(gain), sampleCount(0)
{
}

void RttEstimator2::addMeasurement(time::microseconds measure)
{
  double m = static_cast<double>(measure.count());
  if (sampleCount > 0) {
    double err = m - rttInMicroSec;
    double gErr = err * gain;
    rttInMicroSec += gErr;
  }
  else {
    rttInMicroSec = m;
  }
  sampleCount++;
}

double RttEstimator2::getRttInMilliseconds() const
{
  return rttInMicroSec / (double) 1000.0;
}

}  // namespace fw
}  // namespace nfd