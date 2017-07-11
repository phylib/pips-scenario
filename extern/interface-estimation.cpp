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

#include "interface-estimation.hpp"
#include "core/logger.hpp"

namespace nfd {
namespace fw {

NFD_LOG_INIT("InterfaceEstimation")

InterfaceEstimation::InterfaceEstimation(
    time::milliseconds interestLifetime,
    time::milliseconds calculationWindow): rtt(),
loss(interestLifetime,
    calculationWindow),
bw(calculationWindow)
{
}

void InterfaceEstimation::addSatisfiedInterest(size_t sizeInByte, std::string name)
{
  loss.addSatisfiedInterest(name);
  bw.addPacket(sizeInByte);
}

void InterfaceEstimation::addSentInterest(std::string name)
{
  loss.addSentInterest(name);
}

void InterfaceEstimation::removeSentInterest(std::string name)
{
  loss.removeSentInterest(name);
}

void InterfaceEstimation::addRttMeasurement(time::microseconds durationMicroSeconds)
{
  rtt.addMeasurement(durationMicroSeconds);
}

double InterfaceEstimation::getCurrentValue(RequirementType type)
{
  double returnValue;
  if (type == RequirementType::BANDWIDTH) {
    returnValue = bw.getKBytesPerSecond();
  }
  else if (type == RequirementType::DELAY) {
    if (loss.getLossPercentage() >= 1) {
      returnValue = 1000 * 1000;
    }
    else {
      returnValue = rtt.getRttInMilliseconds();
    }
  }
  else if (type == RequirementType::LOSS) {
    returnValue = loss.getLossPercentage();
  }
  else {
    NFD_LOG_WARN("Invalid type. Should not happen!\n");
    return -1;
  }
  return returnValue;
}

}
// namespace fw
}// namespace nfd