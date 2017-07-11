/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef NFD_DAEMON_FW_PER_PREFIX_INFO_HPP
#define NFD_DAEMON_FW_PER_PREFIX_INFO_HPP

#include "strategy-info.hpp"
#include "strategy-requirements.hpp"
#include <unordered_map>
#include "../face/face.hpp"
#include "interface-estimation.hpp"

namespace nfd {
namespace fw {

/**
 * Measurement information that can be saved and retrieved per-prefix.
 */
class MeasurementInfo : public StrategyInfo
{
public:
  static constexpr int getTypeId()
  {
    return 1012;
  }

  MeasurementInfo() :
      currentWorkingFaceId(-1)
  {
  }

public:
  // A map containing per-face measurements
  std::unordered_map<FaceId, InterfaceEstimation> faceInfoMap;

  // A map where timestamps of sent Interests are saved for RTT measurement.
  std::unordered_map<std::string, time::steady_clock::TimePoint> rttTimeMap;

  // A set containing the names of all the probes that have been redirected (tainted) by this router.
  std::set<std::string> myTaintedProbes;

  // An object which can hold all the hard limits required for this strategy.
  StrategyRequirements req;

  // The ID of the currently best face to use for workload.
  FaceId currentWorkingFaceId;

};

}  //fw
}  //nfd

#endif