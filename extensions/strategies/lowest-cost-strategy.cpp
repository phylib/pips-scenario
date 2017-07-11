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

#include "lowest-cost-strategy.hpp"
#include "core/logger.hpp"
#include "fw/measurement-info.hpp"
#include "fw/algorithm.hpp"
#include "../utils/parameterconfiguration.h"

namespace nfd {
namespace fw {

NFD_LOG_INIT("LowestCostStrategy");

const Name LowestCostStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/lowest-cost/%FD%01/");
NFD_REGISTER_STRATEGY(LowestCostStrategy);

LowestCostStrategy::LowestCostStrategy(Forwarder& forwarder, const Name& name)
 :  Strategy(forwarder, name), 
    ownStrategyChoice(forwarder.getStrategyChoice()),
    taintingCounter(1)
{
  // Setting shared parameters
  PROBE_SUFFIX = ParameterConfiguration::getInstance()->PROBE_SUFFIX;
  PREFIX_OFFSET = ParameterConfiguration::getInstance()->PREFIX_OFFSET;
}

void LowestCostStrategy::afterReceiveInterest(const Face& inFace, 
                                              const Interest& interest,
                                              const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("Íncoming Interest: " << interest.getName());

  // Fetch and prepare the fibEntry
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  // Get the current prefix from the interest name
  std::string currentPrefix = interest.getName().getPrefix(PREFIX_OFFSET).toUri();

  // Set per-prefix parameters
  refreshParameters(currentPrefix);

  // Check if there are noe measurements yet for the current prefix
  if (measurementMap.find(currentPrefix) == measurementMap.end())
  {
    // Create a new MeasurementInfo and place it in the measurementMap under the currentPrefix. 
    MeasurementInfo mi;
    mi.req.setParameter(RequirementType::DELAY, REQUIREMENT_MAXDELAY);
    mi.req.setParameter(RequirementType::LOSS, REQUIREMENT_MAXLOSS);
    mi.req.setParameter(RequirementType::BANDWIDTH, REQUIREMENT_MINBANDWIDTH);
    mi.currentWorkingFaceId = getFaceIdViaBestRoute(nexthops, pitEntry);
    measurementMap[currentPrefix] = mi;
  }

  // Get the ID to the outface that this Interest will be forwarded to
  FaceId selectedOutFaceId = measurementMap[currentPrefix].currentWorkingFaceId;

  // Check if packet is a probe (only probes may be redirected)
  if (interest.getName().toUri().find(PROBE_SUFFIX) != std::string::npos)
  {
    // Determine best outFace (could be another one than currentBestOutFace)
    measurementMap[currentPrefix].currentWorkingFaceId = lookForBetterOutFaceId(nexthops, pitEntry, currentPrefix);
    selectedOutFaceId = measurementMap[currentPrefix].currentWorkingFaceId;

    // Check if packet is untainted (tainted packets must not be redirected or measured)
    if (!interest.isTainted())
    {
      // Check if there is more than one outFace (no need to redirect if no alternatives available)
      if (nexthops.size() >= MIN_NUM_OF_FACES_FOR_TAINTING)
      {
        // Check if this router is allowed to use this probe for monitoring alternative routes 
        if (taintingAllowed() && TAINTING_ENABLED)
        {
          // Mark Interest as tainted, so other routers don't use it or its data packtes for measurements
          // NOTE: const_cast is a hack and should generally be avoided!
          Interest& nonConstInterest = const_cast<Interest&>(interest);
          nonConstInterest.setTainted(true);

          NFD_LOG_INFO("Tainted this interest: " << interest.getName());

          // Remember that this probe was tainted by this router, so the corresponding data can be recognized
          measurementMap[currentPrefix].myTaintedProbes.insert(interest.getName().toUri());

          // Prepare an alternative path for the probe 
          selectedOutFaceId = getAlternativeOutFaceId(measurementMap[currentPrefix].currentWorkingFaceId, nexthops);   

          // Send a NACK back to the previous routers so they don't keep measurement data of the tainted Interest 
          lp::NackHeader nackHeader;
          nackHeader.setReason(lp::NackReason::TAINTED);
          this->sendNack(pitEntry, inFace, nackHeader);

          NFD_LOG_INFO("Send NACK for interest: " << interest.getName() << " on face " << inFace.getId() << " with reason " << nackHeader.getReason());

          // Manually re-insert an in-record for the pit entry, so the Interest can still be sent.
          // NOTE: const_cast is a hack and should generally be avoided!
          Face& nonConstInFace = const_cast<Face&> (inFace);
          pitEntry->insertOrUpdateInRecord(nonConstInFace, interest);
        }
      }
      // Save the probe's sending time in a map for later calculations of rtt. 
      // This is a workaround since "outRecord->getLastRenewed()" somehow doesn't provide the right value. 
      measurementMap[currentPrefix].rttTimeMap[interest.getName().toUri()] = time::steady_clock::now(); 

      // Inform the original estimators (by Klaus Schneider) about the probe
      measurementMap[currentPrefix].faceInfoMap[selectedOutFaceId].addSentInterest(interest.getName().toUri()); 
    }
  } 

  // Check if chosen face is the face the interest came from
  if (selectedOutFaceId == inFace.getId())
  {
    NFD_LOG_INFO("selectedOutFaceId " << selectedOutFaceId << " == inFace " << inFace.getId() << " " << interest.getName());
    selectedOutFaceId = getAlternativeOutFaceId(selectedOutFaceId, nexthops);
  }

  // After everthing else is handled, forward the Interest on the selected face.
  this->sendInterest(pitEntry, getFaceViaId(selectedOutFaceId, nexthops), interest);

  NFD_LOG_DEBUG("Sending Interest " << interest.getName() << " on face " << selectedOutFaceId);

  // Printing current measurement status to console. 
  InterfaceEstimation& faceInfo1 = measurementMap[currentPrefix].faceInfoMap[measurementMap[currentPrefix].currentWorkingFaceId];
  NFD_LOG_INFO("Interest " << interest.getName() << " forwarded on face " << selectedOutFaceId); 
  NFD_LOG_INFO("Face (working path): "    << measurementMap[currentPrefix].currentWorkingFaceId 
                << " - delay: "  << faceInfo1.getCurrentValue(RequirementType::DELAY)  
                << "ms, loss: " << faceInfo1.getCurrentValue(RequirementType::LOSS) * 100  
                << "%, bw: "    << faceInfo1.getCurrentValue(RequirementType::BANDWIDTH)); 
  InterfaceEstimation& faceInfo2 = measurementMap[currentPrefix].faceInfoMap[selectedOutFaceId]; 
  NFD_LOG_INFO("Face (alternative path): "    << selectedOutFaceId
                << " - delay: "  << faceInfo2.getCurrentValue(RequirementType::DELAY)  
                << "ms, loss: " << faceInfo2.getCurrentValue(RequirementType::LOSS) * 100  
                << "%, bw: "    << faceInfo2.getCurrentValue(RequirementType::BANDWIDTH)); 
  // std::cout << std::endl;



  return;
}


FaceId LowestCostStrategy::lookForBetterOutFaceId(const fib::NextHopList& nexthops,
                                                  const shared_ptr<pit::Entry> pitEntry,
                                                  std::string currentPrefix)
{
  // Check if there is only one available face anyway.
  if (nexthops.size() <= 2)
  {
    NFD_LOG_INFO("Only one face available. Using bestRoute." << pitEntry->getInterest().getName());
    return getFaceIdViaBestRoute(nexthops, pitEntry);
  }
  double delayLimit = measurementMap[currentPrefix].req.getLimit(RequirementType::DELAY); 
  double lossLimit = measurementMap[currentPrefix].req.getLimit(RequirementType::LOSS);
  double bandwidthLimit = measurementMap[currentPrefix].req.getLimit(RequirementType::BANDWIDTH);
  double currentDelay = measurementMap[currentPrefix].faceInfoMap[measurementMap[currentPrefix].currentWorkingFaceId].getCurrentValue(RequirementType::DELAY); 
  double currentLoss = measurementMap[currentPrefix].faceInfoMap[measurementMap[currentPrefix].currentWorkingFaceId].getCurrentValue(RequirementType::LOSS); 
  double currentBandwidth = measurementMap[currentPrefix].faceInfoMap[measurementMap[currentPrefix].currentWorkingFaceId].getCurrentValue(RequirementType::BANDWIDTH);

  // Check if current working path measurements are still uninitialised
  if (currentDelay == 10 && currentLoss == 0 && currentBandwidth == 0)
  { 
    NFD_LOG_INFO ("Measurements still uninitialised. Staying on current working path.");
    return measurementMap[currentPrefix].currentWorkingFaceId;
  }

  // Check if current working path underperforms
  if (currentDelay > delayLimit || currentLoss > lossLimit || currentBandwidth < bandwidthLimit)
  {
    NFD_LOG_INFO("Current face underperforms: Face " << measurementMap[currentPrefix].currentWorkingFaceId << ", " << currentDelay << ", " << currentLoss * 100 << "%, " << currentBandwidth);
    // Find potential alternative and get its performance
    FaceId alternativeOutFaceId = getAlternativeOutFaceId(measurementMap[currentPrefix].currentWorkingFaceId, nexthops);
    double alternativeDelay = measurementMap[currentPrefix].faceInfoMap[alternativeOutFaceId].getCurrentValue(RequirementType::DELAY); 
    double alternativeLoss = measurementMap[currentPrefix].faceInfoMap[alternativeOutFaceId].getCurrentValue(RequirementType::LOSS); 
    double alternativeBandwidth = measurementMap[currentPrefix].faceInfoMap[alternativeOutFaceId].getCurrentValue(RequirementType::BANDWIDTH);
    
    // Check if alternative performs well enough
    if (alternativeDelay <= delayLimit && alternativeLoss <= lossLimit && alternativeBandwidth >= bandwidthLimit)
    {
      if (canForwardToLegacy(*pitEntry, getFaceViaId(alternativeOutFaceId, nexthops))) 
      { 
        NFD_LOG_INFO("Well performing alternative face found: " << alternativeOutFaceId);
        return alternativeOutFaceId; 
      }
    }
    else 
    {
      /* 
       * If alternative also underperforms, take the next alternative and hope for the best 
       * (since there will be no performance data available yet)
       */
      if (canForwardToLegacy(*pitEntry, getFaceViaId(alternativeOutFaceId, nexthops))) 
      { 
        NFD_LOG_INFO("Taking next best alternative out of desperation: " << getAlternativeOutFaceId(alternativeOutFaceId, nexthops) << " " << pitEntry->getInterest().getName());
        return getAlternativeOutFaceId(alternativeOutFaceId, nexthops); 
      }      
    }
  } 
  // If current path performs well enough, just stay on it.
  NFD_LOG_INFO("Current working path performs well enough. Staying on it. " << measurementMap[currentPrefix].currentWorkingFaceId);
  return measurementMap[currentPrefix].currentWorkingFaceId;
}


FaceId LowestCostStrategy::getFaceIdViaBestRoute( const fib::NextHopList& nexthops, 
                                                  const shared_ptr<pit::Entry> pitEntry)
{
  // Copied and adjusted from the bestRoute algorithm
  for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {
    if (canForwardToLegacy(*pitEntry, it->getFace())) {
      return it->getFace().getId();
    }
  }

  // If everything else fails, just use the first face.
  return nexthops[0].getFace().getId();
}


FaceId LowestCostStrategy::getAlternativeOutFaceId( FaceId outFaceId, 
                                                    const fib::NextHopList& nexthops)
{
  if (nexthops.size() > 1)
  {
    bool faceFound = false;
    int iterations = 0;
    while (iterations < 2) {
      for (auto n : nexthops) 
      { 
        if (faceFound) {return n.getFace().getId();} 
        if (n.getFace().getId() == outFaceId) {faceFound = true;}
      }
      iterations++;
    }
  }
  return outFaceId;
}

Face& LowestCostStrategy::getFaceViaId( FaceId faceId, 
                                        const fib::NextHopList& nexthops)
{
  // Try to find a face with the given ID in nexthops
  for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {
    if (it->getFace().getId() == faceId)
    {
      return it->getFace();
    }
  }
  // If no face was found, just use the first.
  NFD_LOG_WARN("Face " << (int)faceId << " was not found in nexthops. Returned face " << (int) nexthops[0].getFace().getId() << " instead.");
  return nexthops[0].getFace();
}

bool LowestCostStrategy::taintingAllowed()
{
  if (taintingCounter >= MAX_TAINTED_PROBES_PERCENTAGE) {
    taintingCounter = 1;
    return true;
  }
  else {
    taintingCounter++;
    return false;
  }
}

void LowestCostStrategy::refreshParameters(std::string currentPrefix) 
{
  //Setting parametes with values from ParameterConfiguration;
  TAINTING_ENABLED = ParameterConfiguration::getInstance()->getParameter("TAINTING_ENABLED", currentPrefix);
  MIN_NUM_OF_FACES_FOR_TAINTING = ParameterConfiguration::getInstance()->getParameter("MIN_NUM_OF_FACES_FOR_TAINTING", currentPrefix);
  MAX_TAINTED_PROBES_PERCENTAGE = ParameterConfiguration::getInstance()->getParameter("MAX_TAINTED_PROBES_PERCENTAGE", currentPrefix);
  REQUIREMENT_MAXDELAY = ParameterConfiguration::getInstance()->getParameter("REQUIREMENT_MAXDELAY", currentPrefix);
  REQUIREMENT_MAXLOSS = ParameterConfiguration::getInstance()->getParameter("REQUIREMENT_MAXLOSS", currentPrefix);
  REQUIREMENT_MINBANDWIDTH = ParameterConfiguration::getInstance()->getParameter("REQUIREMENT_MINBANDWIDTH", currentPrefix);
  HYSTERESIS_PERCENTAGE = ParameterConfiguration::getInstance()->getParameter("HYSTERESIS_PERCENTAGE", currentPrefix);
  RTT_TIME_TABLE_MAX_DURATION = time::milliseconds((int)ParameterConfiguration::getInstance()->getParameter("RTT_TIME_TABLE_MAX_DURATION", currentPrefix));
}


void LowestCostStrategy::beforeSatisfyInterest( const shared_ptr<pit::Entry>& pitEntry,
                                                const Face& inFace, 
                                                const Data& data)
{
  NFD_LOG_DEBUG("Received data: " << data.getName());

  // Get the current prefix from the data name
  std::string currentPrefix = data.getName().getPrefix(PREFIX_OFFSET).toUri();

  // Set per-prefix parameters
  refreshParameters(currentPrefix);

  // Check if incoming data is probe data
  if (data.getName().toUri().find(PROBE_SUFFIX) != std::string::npos)
  {
    // Check if it's an answer to one of the probes tainted by this router
    auto myTaintedProbesIterator = measurementMap[currentPrefix].myTaintedProbes.find(data.getName().toUri());
    bool taintedByThisRouter = (myTaintedProbesIterator != measurementMap[currentPrefix].myTaintedProbes.end()) ? true : false;

    // Check if usable for measurement (tainted by this router or not tainted at all)
    if (taintedByThisRouter || !data.isTainted())
    {
      if (taintedByThisRouter)
      {
        // Forget about the corresponding tainted probe (since it is satisfied now)
        measurementMap[currentPrefix].myTaintedProbes.erase(myTaintedProbesIterator);
        NFD_LOG_INFO("Removed " << data.getName() << "from myTaintedProbes.");

        // @todo: Find a way to stop the data packet from being forwarded any further.
      }
      // Inform loss estimator
      InterfaceEstimation& faceInfo = measurementMap[currentPrefix].faceInfoMap[inFace.getId()];
      faceInfo.addSatisfiedInterest(data.getContent().value_size(), data.getName().toUri());
      pit::OutRecordCollection::const_iterator outRecord = pitEntry->getOutRecord(inFace);

      // Check if not already satisfied by another upstream
      if (!pitEntry->getInRecords().empty() && outRecord != pitEntry->getOutRecords().end()) 
      {
        // There is an in and outrecord --> inform RTT estimator
        time::steady_clock::Duration rtt = time::steady_clock::now() - measurementMap[currentPrefix].rttTimeMap[data.getName().toUri()];
        faceInfo.addRttMeasurement(time::duration_cast < time::microseconds > (rtt));
        measurementMap[currentPrefix].rttTimeMap.erase(data.getName().toUri()); 

        // Delete every entry in rttTimeMap that is older than a certain threshold.
        for ( auto it = measurementMap[currentPrefix].rttTimeMap.begin(); it != measurementMap[currentPrefix].rttTimeMap.end();) 
        {
          (time::steady_clock::now()-it->second > RTT_TIME_TABLE_MAX_DURATION) ? it=measurementMap[currentPrefix].rttTimeMap.erase(it) : it++ ;
        } 
      }   
    }    
    else 
    {
      /*
      * @todo: Find a way to determine if this router is before or after the "tainter", since it could theoretically 
      *        use tainted probes for measurement as long as it is a router after the tainter.
      */
    }


  } 
}

void 
LowestCostStrategy::afterReceiveNack( const Face& inFace, 
                                      const lp::Nack& nack, 
                                      const shared_ptr<pit::Entry>& pitEntry) 
{
  NFD_LOG_DEBUG("Received NACK for " << pitEntry->getInterest().getName() << " with NackReason = " << nack.getReason());

  // Get the current prefix from the pit entry
  std::string currentPrefix = pitEntry->getInterest().getName().getPrefix(PREFIX_OFFSET).toUri();

  // Set per-prefix parameters
  refreshParameters(currentPrefix);

  if (nack.getReason() == lp::NackReason::TAINTED)
  {
      /*
       * Cancel measurements for tainted data packet (so that measurements are not skewed by 'missing packtets'). 
       * Delay: Just dont calculate rtt, entries will drop out of the custom list on their own
       * Loss: Omit "addSatisfiedInterest" and remove the corresponding entry from the estimator
       * Bandwith: Omit "addSatisfiedInterest"
       */ 
      measurementMap[currentPrefix].faceInfoMap[inFace.getId()].removeSentInterest(pitEntry->getInterest().getName().toUri());
      NFD_LOG_INFO("Removed measurements for " << pitEntry->getInterest().getName());

      // Forward NACK further back to the previous routers so they don't keep measurement data of the tainted Interest either.
      this->sendNack(pitEntry, pitEntry->getInRecords().begin()->getFace(), nack.getHeader());
  }
}

}  // namespace fw
}  // namespace nfd
