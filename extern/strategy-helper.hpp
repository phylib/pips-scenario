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

#ifndef NFD_DAEMON_FW_STRATEGY_HELPER_HPP
#define NFD_DAEMON_FW_STRATEGY_HELPER_HPP

#include <map>
#include <string>
#include <tuple>
#include "../table/measurements-accessor.hpp"
#include "measurement-info.hpp"

namespace nfd {
namespace fw {

class StrategyHelper
{

public:

  StrategyHelper();

  /**
   * Parses a parameter string to a map of parameter attribute names to attribute values.
   *
   * \param parameterString is of the format "param1=value1,param2=value2,...,paramn=valuen"
   */
  static std::map<std::string, std::string> getParameterMap(std::string parameterString);

  /**
   * Finds a measurement for the given interest name in a longest prefix match.
   */
  static std::tuple<Name, shared_ptr<MeasurementInfo>> findPrefixMeasurements(
      const Interest& interest, const MeasurementsAccessor& measurements);

  /**
   * Adds a prefix measurement one level upwards of the interest name.
   */
  static shared_ptr<MeasurementInfo> addPrefixMeasurements(const Interest& interest,
      MeasurementsAccessor& measurements);

  /**
   * Performs probing every x th packet.
   *
   * \returns true for every 10 th method run.
   */
  bool probingDue();

private:
  int probingCounter;

};

}  // namespace fw
}  // namespace nfd

#endif
