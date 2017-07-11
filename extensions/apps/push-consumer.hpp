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

#ifndef PUSH_CONSUMER_H
#define PUSH_CONSUMER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ndnSIM/apps/ndn-consumer.hpp"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief Ndn application for sending out Interest packets at a "constant" rate (Poisson process)
 */
class PushConsumer : public Consumer {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   * Sets up randomizer function and packet sequence number
   */
  PushConsumer();
  virtual ~PushConsumer();

  // From Consumer
  virtual void
  OnData(shared_ptr<const Data> contentObject);

protected:
  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  virtual void
  ScheduleNextPacket();

  /**
   * @brief Constructs the probe Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  virtual void
  ScheduleNextProbe();

  /**
   * @brief Set type of frequency randomization
   * @param value Either 'none', 'uniform', or 'exponential'
   */
  void
  SetRandomize(const std::string& value);

  /**
   * @brief Get type of frequency randomization
   * @returns either 'none', 'uniform', or 'exponential'
   */
  std::string
  GetRandomize() const;
  
  void
  SendPacket();

  void
  SendProbe();

protected:
  double m_frequency; // Frequency of interest packets (in hertz)
  double m_probeFrequency; // Frequency of probe packets per second
  bool m_firstTime;
  Ptr<RandomVariableStream> m_random;
  std::string m_randomType;
  const std::string PROBE_SUFFIX = "/probe";
  
  Time m_refreshInterval; 
  std::string m_use_refresh_pi = "false";
  uint32_t m_qci = 0;
};

} // namespace ndn
} // namespace ns3

#endif
