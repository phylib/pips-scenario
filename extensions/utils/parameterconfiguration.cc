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

#include "parameterconfiguration.h"

ParameterConfiguration* ParameterConfiguration::instance = NULL;

ParameterConfiguration::ParameterConfiguration()
{
  setParameter("TAINTING_ENABLED", P_TAINTING_ENABLED);
  setParameter("MIN_NUM_OF_FACES_FOR_TAINTING", P_MIN_NUM_OF_FACES_FOR_TAINTING);
  setParameter("MAX_TAINTED_PROBES_PERCENTAGE", P_MAX_TAINTED_PROBES_PERCENTAGE);
  setParameter("REQUIREMENT_MAXDELAY", P_REQUIREMENT_MAXDELAY);
  setParameter("REQUIREMENT_MAXLOSS", P_REQUIREMENT_MAXLOSS);
  setParameter("REQUIREMENT_MINBANDWIDTH", P_REQUIREMENT_MINBANDWIDTH);
  setParameter("RTT_TIME_TABLE_MAX_DURATION", P_RTT_TIME_TABLE_MAX_DURATION);
}


void ParameterConfiguration::setParameter(std::string param_name, double value, std::string prefix)
{
  prefixMap[prefix][param_name] = value;
}

double ParameterConfiguration::getParameter(std::string param_name, std::string prefix)
{
  return prefixMap[prefix][param_name];
}

ParameterConfiguration *ParameterConfiguration::getInstance()
{
  if(instance == NULL)
    instance = new ParameterConfiguration();

  return instance;
}