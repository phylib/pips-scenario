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

#include "loss-estimator-time-window.hpp"
#include "core/logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>

namespace nfd {
namespace fw {

NFD_LOG_INIT("LossEstimator");

LossEstimatorTimeWindow::LossEstimatorTimeWindow(time::steady_clock::duration interestLifetime,
    time::steady_clock::duration windowSize) :
    m_interestLifetime(interestLifetime), m_windowSize(windowSize)
{
  if (m_windowSize <= m_interestLifetime) {
    throw std::runtime_error("Window size must be greater than interest lifetime!");
  }
}

void LossEstimatorTimeWindow::addSentInterest(const std::string& name)
{
  const time::steady_clock::TimePoint now = time::steady_clock::now();
  auto n = unknownMap.insert(std::make_pair(name, now));

  NFD_LOG_TRACE("unknownMap.insert(" << name << ")");

  if (n.second == false) {
    NFD_LOG_WARN("Duplicate insertion: " << name << " Should not happen!\n");
    // throw std::runtime_error("Duplicate insertion of sent interest!");
  }

}

void LossEstimatorTimeWindow::removeSentInterest(const std::string& name)
{
/*  NFD_LOG_DEBUG("List before: v");
  for (auto const& x : unknownMap)
  {
    std::cout << x.first  // string (key)
              << ':' 
              << x.second // string's value 
              << std::endl ;
  }*/
  
  int count = unknownMap.erase(name);
  NFD_LOG_TRACE("Removed " << count << " interests from list: " << name);

/*  for (auto const& x : unknownMap)
  {
    std::cout << x.first  // string (key)
              << ':' 
              << x.second // string's value 
              << std::endl ;
  }
  NFD_LOG_DEBUG("List after: ^");
  std::cout << std::endl;*/
}

void LossEstimatorTimeWindow::addSatisfiedInterest(const std::string& name)
{
  bool found = false;

  for (auto n : unknownMap) {
    // Add new data
    if (n.first == name) {
      NFD_LOG_TRACE("Adding found interest!: " << name);
      found = true;
      lossMap.insert(std::make_pair(n.second, PacketType::FUTURESATISFIED));
      NFD_LOG_TRACE("lossMap.insert(" << n.second << ",FUTURESATISFIED)");
      unknownMap.erase(n.first);
      NFD_LOG_TRACE("unknownMap.erase(" << n.first << ")");
    }
  }
  if (found == false) {
    NFD_LOG_TRACE(
        "Interest " << name
            << " not found! Data packet returned after interest lifetime exceeded!");
    // Still add the data packet?
    lossMap.insert(std::make_pair(time::steady_clock::now(), PacketType::FUTURESATISFIED));
  }
}

double LossEstimatorTimeWindow::getLossPercentage()
{
  NFD_LOG_DEBUG("Calling updateMeasurements()");
  // Ensure measurements are up to date before doing any calculations
  updateMeasurements();

  double perc;

  // Return 0 if the map is empty
  if (lossMap.empty()) {
    NFD_LOG_TRACE("LossMap empty!");
    perc = 0;
  }
  else {
    int satisfied = 0;
    int lost = 0;
    for (auto n : lossMap) {
      if (n.second == PacketType::SATISFIED) {
        satisfied++;
      }
      if (n.second == PacketType::LOST) {
        lost++;
      }
    }

    // Return 0 if only FUTURESATISFIED packets are in the map
    if (lost + satisfied == 0) {
      NFD_LOG_TRACE("Only FutureSatisfied!");
      perc = 0;
    }
    else {
      perc = (double) lost / (double) (lost + satisfied);
    }
  }

  NFD_LOG_TRACE("Loss Percentage: " << perc);
  return perc;
}

void
LossEstimatorTimeWindow::updateMeasurements()
{
  NFD_LOG_DEBUG("updateMeasurements() was called.");
  
  const time::steady_clock::TimePoint now = time::steady_clock::now();

  // Turning FUTURESATISFIED into SATISFIED (when the interest lifetime is exceeded)
  for (auto n : lossMap) {
    if (now > n.first + m_interestLifetime && n.second == PacketType::FUTURESATISFIED) {
      time::steady_clock::time_point temp = n.first;
      lossMap.erase(n.first);
      lossMap.insert(std::make_pair(temp, PacketType::SATISFIED));
      NFD_LOG_TRACE("Turning " << n.first << " from FUTURESATISFIED into SATISFIED (lossmap)");
    }
  }

  // Add lost interests
  for (auto n : unknownMap) {
    if (now > n.second + m_interestLifetime) {
      lossMap.insert(std::make_pair(n.second, PacketType::LOST));
      unknownMap.erase(n.first);
      NFD_LOG_TRACE("Interest " << n.first << " marked as LOST.");
      NFD_LOG_TRACE("lossMap.insert(" << n.second << ",LOST.)");
    }
  }

  // Remove packets that fall out of window size
  time::steady_clock::TimePoint lastValidInterests = now - m_windowSize;
  lossMap.erase(lossMap.begin(), lossMap.upper_bound(lastValidInterests));

  // Set timer to ensure this method is called periodically
  setRefreshTimer();
}

void
LossEstimatorTimeWindow::setRefreshTimer()
{
time::nanoseconds refreshTime = time::milliseconds(1000); // 1 second
  scheduler::cancel(m_refreshTimer);
  m_refreshTimer = scheduler::schedule(refreshTime, bind(&LossEstimatorTimeWindow::updateMeasurements, this));
}

}  // namespace fw
}  // namespace nfd