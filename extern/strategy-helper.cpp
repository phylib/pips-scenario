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

#include "strategy-helper.hpp"
#include "core/logger.hpp"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/detail/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/assert.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/name.hpp>
#include <cstdbool>
#include <memory>
#include <vector>
#include "../table/measurements-entry.hpp"

namespace nfd {
namespace fw {

NFD_LOG_INIT ("StrategyHelper");

StrategyHelper::StrategyHelper() :
    probingCounter(1)
{
}

std::map<std::string, std::string> StrategyHelper::getParameterMap(std::string parameters)
{
  NFD_LOG_TRACE("Parsing parameters!");

  std::map < std::string, std::string > outputMap;

// Replace ASCII elements
  boost::replace_all(parameters, "%2C", ",");
  boost::replace_all(parameters, "%3D", "=");

  std::vector < std::string > paramVector;
  boost::split(paramVector, parameters, boost::is_any_of(","));

  for (auto substring : paramVector) {
    std::vector < std::string > substringVector;
    boost::split(substringVector, substring, boost::is_any_of("="));

    std::string key = substringVector.front();
    std::string value = substringVector.back();
    outputMap[key] = value;
  }

  return outputMap;
}

bool StrategyHelper::probingDue()
{
  if (probingCounter >= 10) {
    probingCounter = 1;
    return true;
  }
  else {
    probingCounter++;
    return false;
  }
}

std::tuple<Name, shared_ptr<MeasurementInfo>> StrategyHelper::findPrefixMeasurements(
    const Interest& interest, const MeasurementsAccessor& measurements)
{
  shared_ptr < measurements::Entry > me = shared_ptr < measurements::Entry >(measurements.findLongestPrefixMatch(interest.getName()));
  if (me == nullptr) {
    return std::forward_as_tuple(Name(), nullptr);
  }
  shared_ptr < MeasurementInfo > mi = shared_ptr < MeasurementInfo >(me->getStrategyInfo<MeasurementInfo>());
  return std::forward_as_tuple(me->getName(), mi);
}

shared_ptr<MeasurementInfo> StrategyHelper::addPrefixMeasurements(const Interest& interest,
    MeasurementsAccessor& measurements)
{
  shared_ptr < measurements::Entry > me;
  if (interest.getName().size() >= 1) {
    // Save info one step up.
    me = shared_ptr < measurements::Entry >(measurements.get(interest.getName().getPrefix(-1)));
  }
  // parent of Interest Name is not in this strategy, or Interest Name is empty
  if (me == nullptr) {
    me = shared_ptr < measurements::Entry >(measurements.get(interest.getName()));
  }

  return shared_ptr < MeasurementInfo >(me->insertStrategyInfo<MeasurementInfo>().first);
}

}  // namespace nfd
}  // namespace fw
