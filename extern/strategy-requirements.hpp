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

#ifndef NFD_DAEMON_FW_STRATEGY_REQUIREMENTS_HPP
#define NFD_DAEMON_FW_STRATEGY_REQUIREMENTS_HPP

#include <cstdbool>
#include <map>
#include <utility>
#include <set>
#include <string>

namespace nfd {
namespace fw {

enum class RequirementType
{
  BANDWIDTH, COST, DELAY, LOSS
};

/**
 * A class that represents the requirements (parameters) that a strategy currently holds.
 */
class StrategyRequirements
{
public:

  /**
   * Constructor.
   *
   * @param supportedRequirements A set that contains all the requirement types
   * that the strategy supports.
   *
   */
  StrategyRequirements(std::set<RequirementType> supportedRequirements = {
      RequirementType::BANDWIDTH, RequirementType::COST, RequirementType::DELAY,
      RequirementType::LOSS });

  /**
   * Checks if the given requirement is an upward attribute or not.
   *
   * Upward attributes: BANDWIDTH (a higher value is preferable)
   * Downward attributes: LOSS, DELAY, COST (a lower value is preferable)
   *
   * @param type The type of requirement which should be checked.
   * @returns true if the given requirement is an upward attribute.
   */
  static bool isUpwardAttribute(RequirementType type);

  /**
   * Takes a string of parameters and adds the corresponding requirement attributes and values.
   *
   * @param parameterString Has the syntax "p1=v1,...pn=vn" or "p1=vl1-vl2,...".
   * Valid names for pn are "maxloss", "mindelay", "minbw" and "mincost".
   *
   * @returns true if at least one parameter was valid (supported and contained in parameterString).
   * @returns false otherwise.
   */
  bool parseParameters(std::string parameterString);

  /**
   * Allows for setting the requirement limits directly via value, instead of having to parse a string.
   *
   * @param type The type of requirement that should be set. Valid types are BANDWIDTH, COST, DELAY and LOSS.
   * @param value The value the requirement should have.
   */ 
  void setParameter(RequirementType type, double value);

  /**
   * Returns the limits for one specific requirement type.
   *
   * @param type The type of the requirement for which the limits are requested.
   * @returns std::pair<lower limit, upper limit>
   * @returns the same value for both if there is only one limit.
   * @returns std::pair<-1,-1> if the type is not instantiated.
   */
  std::pair<double, double> getLimits(RequirementType type);

  /**
   * @param type The type of the requirement for which the limit is requested.
   * @returns the limit for one specific requirement type.
   * @returns The lower limit if they differ and logs a warning message.
   */
  double getLimit(RequirementType type);

  /**
   * @param type The type of the requirement that should be checked.
   * @returns true if the requirement type is supported and has an assigned value.
   */
  bool contains(RequirementType type);

  /**
   * @returns a set with all supported and instantiated requirement types.
   */
  std::set<RequirementType> getOwnTypes() const;

private:

  std::map<RequirementType, std::pair<double, double>> requirementMap;
  std::set<RequirementType> supportedRequirements;
  std::set<RequirementType> initializedRequirements;

};

}  // namespace fw
}  // namespace nfd

#endif
