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

#include "strategy-requirements.hpp"
#include "strategy-helper.hpp"
#include "core/logger.hpp"
#include <vector>
#include <boost/algorithm/string.hpp>

NFD_LOG_INIT("StrategyRequirements")

namespace nfd {

  namespace fw {

    StrategyRequirements::StrategyRequirements(std::set<RequirementType> supportedRequirements) :
    supportedRequirements(supportedRequirements), initializedRequirements(std::set<RequirementType>())
    {
    }

    bool StrategyRequirements::contains(RequirementType type)
    {
      if (requirementMap.count(type) == 0) {
        return false;
      }
      else {
        return true;
      }
    }

    double StrategyRequirements::getLimit(RequirementType type) {
      double first = getLimits(type).first;
      double second = getLimits(type).second;
      if (first != second) {
        NFD_LOG_WARN("Different limits. Returning lower one!");
      }
      return getLimits(type).first;
    }

    std::pair<double, double> StrategyRequirements::getLimits(RequirementType type)
    {
      if (requirementMap.count(type) != 0) {
        return requirementMap[type];
      }
      else {
        return std::pair<double, double> {-1, -1};
      }
    }

    std::set<RequirementType> StrategyRequirements::getOwnTypes() const
    {
      std::set < RequirementType > set;
      for (auto n : requirementMap) {
        set.insert(n.first);
      }
      return set;
    }

    bool StrategyRequirements::isUpwardAttribute(RequirementType type)
    {
      if (type == RequirementType::BANDWIDTH) {
        return true;
      }
      else {
        return false;
      }
    }

    bool StrategyRequirements::parseParameters(std::string parameterString)
    {
      std::map < std::string, std::string > paramStringMap = StrategyHelper::getParameterMap(
          parameterString);

      RequirementType currentType;

      bool foundSupported = false;
      bool found = true;

      for (auto p : paramStringMap) {
        std::string s = p.first;
        if (s.find("maxloss") != std::string::npos) {
          currentType = RequirementType::LOSS;
        }
        else if (s.find("maxdelay") != std::string::npos) {
          currentType = RequirementType::DELAY;
        }
        else if (s.find("maxcost") != std::string::npos) {
          currentType = RequirementType::COST;
        }
        else if (s.find("minbw") != std::string::npos) {
          currentType = RequirementType::BANDWIDTH;
        }
        else {
          found = false;
          NFD_LOG_WARN("Unknown parameter: " << s);
        }

        if (found && supportedRequirements.find(currentType) != supportedRequirements.end()) {
          foundSupported = true;
          if (p.second.find("-") != std::string::npos) {
            std::vector < std::string > vector2;
            boost::split(vector2, p.second, boost::is_any_of("-"));
            requirementMap[currentType] = std::make_pair(std::stod(vector2[0]), std::stod(vector2[1]));
          }
          else {
            requirementMap[currentType] = std::make_pair(std::stod(p.second), std::stod(p.second));
          }

        }

      }

      return foundSupported;
    }

    void StrategyRequirements::setParameter(RequirementType type, double value)
    {
      requirementMap[type] = std::make_pair(value, value);
    }

  }  // namespace fw
}
