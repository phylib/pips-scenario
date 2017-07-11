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

#ifndef PARAMETERCONFIGURATION_H
#define PARAMETERCONFIGURATION_H

#include <cstddef>
#include <map>
#include <string>

//per-prefix parameters
#define P_TAINTING_ENABLED              1      // specifies if probes will be forwarded or not; 1=true, 0=false; 
#define P_MIN_NUM_OF_FACES_FOR_TAINTING 3      // the minimum number of faces a node must have to redirect probes
#define P_MAX_TAINTED_PROBES_PERCENTAGE 10     // percentage of working path probes that may be redirected
#define P_REQUIREMENT_MAXDELAY          200.0  // maximum tolerated delay in milliseconds
#define P_REQUIREMENT_MAXLOSS           0.1    // maximum tolerated loss in percentage
#define P_REQUIREMENT_MINBANDWIDTH      0.0    // minimum tolerated bandwith in Kbps
#define P_RTT_TIME_TABLE_MAX_DURATION   1000   // maximum time (in milliseconds) an entry is kept in the rttMap before being erased

/**
 * The ParameterConfiguration class is used to set/get parameters to configure the lowest-cost-strategy.
 *
 * @note The class uses a singleton pattern.
 */
class ParameterConfiguration
{
public:
  //shared parameters (not per-prefix):
  std::string APP_SUFFIX = "/app"; // suffix that helps identifying persistent Interests
  std::string PROBE_SUFFIX = "/probe"; // suffix that helps identifying probes
  int PREFIX_OFFSET = 1; // number of name components which are considered as prefix


  /**
   * @returns the singleton instance.
   */
  static ParameterConfiguration* getInstance();

  /**
   * Sets a parameter
   *
   * @param param_name The name of the parameter.
   * @param value The value of the parameter.
   * @param prefix The parameter should be set for (optional). 
   */
  void setParameter(std::string param_name, double value, std::string prefix = "/");

  /**
   * Gets a parameter.
   *
   * @param para_name The name of the parameter.
   * @param prefix The parameter should fetched from (optional).
   * @return
   */
  double getParameter(std::string param_name, std::string prefix = "/");

protected:  
  ParameterConfiguration();

  std::map<std::string, double> typedef ParameterMap; // <param name, param value>

  // The singleton instance.
  static ParameterConfiguration* instance;

  // A map containing parameters and their values.
  ParameterMap pmap;

  // A map containing one ParameterMap per prefix.
  std::map<std::string, ParameterMap> prefixMap;


};

#endif // PARAMETERCONFIGURATION_H
