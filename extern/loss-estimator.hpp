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

#ifndef NFD_DAEMON_FW_LOSS_ESTIMATOR_HPP
#define NFD_DAEMON_FW_LOSS_ESTIMATOR_HPP

#include "common.hpp"

namespace nfd {
namespace fw {

/**
 * \brief Interface for all Loss Estimator classes
 */
class LossEstimator
{
public:

  virtual ~LossEstimator()
  {
  }

  /**
   * Adds one sent interest packet
   */
  virtual void addSentInterest(const std::string& name) = 0;

  /**
   * Adds one satisfied interest packet (= received data packet)
   */
  virtual void addSatisfiedInterest(const std::string& name) = 0;

  /**
   * Returns the loss percentage as value between 0 and 1.
   */
  virtual double getLossPercentage() = 0;

};

}  // namespace fw
}  // namespace nfd

#endif // NFD_DAEMON_FW_LOSS_ESTIMATOR_HPP

