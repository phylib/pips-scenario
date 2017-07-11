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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"
#include "../extensions/utils/parameterconfiguration.h"
#include "../extensions/strategies/lowest-cost-strategy.hpp"

#include "../extensions/tracers/push-tracer.hpp"
#include "../extensions/tracers/ndn-l3-packet-tracer.hpp"

using ns3::ndn::StrategyChoiceHelper;

namespace ns3 {

int
main(int argc, char* argv[])
{
  // Variables
  uint32_t simTime = 10 * 60* 1000; // Simtime in milliseconds (10 minutes)
  
  // Parameters
  std::string queue = "DropTail_Bytes";
  std::string forwardingStrategy = "lowest-cost";
  std::string logDir = "results/";
  std::string approach = "push";
  std::string piRefreshFrequency = "1.5s";
  std::string linkErrorParam = "5";
  std::string linkFailureDuration = "120";
  std::string linkFailureDurationVariation = "0.2";
  std::string appSuffix = "/app";
  std::string probeSuffix = "/probe";
  int prefixOffset = 1;
  int taintingEnabled = 1;
  int minNumOfFacesForTainting = 3;
  int maxTaintedProbesPercentage = 10;
  double requirementMaxDelay = 200.0;
  double requirementMaxLoss = 0.2;
  double requirementMinBandwidth = 0.0;
  int rttTimeTableMaxDuration = 1000;
  std::string topologyFile = "scenarios/topologies/PIPS-topology.txt";

  // Defining main prefixes
  std::string prefixA = "/dst1";
  std::string prefixB = "/dst2";

  // Defining combined prefixes
  std::string prefixA_App = prefixA + appSuffix;
  std::string prefixA_Probe = prefixA + probeSuffix;
  std::string prefixB_App = prefixB + appSuffix;
  std::string prefixB_Probe = prefixB + probeSuffix;

  // Read Parameters
  CommandLine cmd;
  cmd.AddValue("queueName", "Name of the queue to use", queue);
  cmd.AddValue("forwardingStrategy", "Used forwarding strategy on all nodes", forwardingStrategy);
  cmd.AddValue("logDir", "Folder where logfiles are stored", logDir);
  cmd.AddValue("approach", "Approach to simulate (push|prerequest|standard). Default: push", approach);
  cmd.AddValue("piRefreshFrequency", "Number of Refresh Persistent Interests per Second", piRefreshFrequency);
  cmd.AddValue("linkErrors", "Number of link errors during simulation", linkErrorParam);
  cmd.AddValue("linkFailureDuration", "Duration of link failures in seconds", linkFailureDuration);
  cmd.AddValue("linkFailureDurationVariation", "Variation of linkFailureDuration in percent", linkFailureDurationVariation);
  cmd.AddValue("appSuffix", "Suffix for routing payload", appSuffix);
  cmd.AddValue("probeSuffix", "Suffix for routing probes", probeSuffix);
  cmd.AddValue("prefixOffset", "Number of name components in 'prefixA'/'prefixB'", prefixOffset);
  cmd.AddValue("taintingEnabled", "Enables/disables redirecting of probes", taintingEnabled);
  cmd.AddValue("minNumOfFacesForTainting", "Number of faces a node must have for tainting", minNumOfFacesForTainting);
  cmd.AddValue("maxTaintedProbesPercentage", "Percentage of probes that may be redirected", maxTaintedProbesPercentage);
  cmd.AddValue("requirementMaxDelay", "Maximum delay allowed for well-performing paths (non-inclusive)", requirementMaxDelay);
  cmd.AddValue("requirementMaxLoss", "Maximum loss allowed for well-performing paths (non-inclusive)", requirementMaxLoss);
  cmd.AddValue("requirementMinBandwidth", "Minimum bandwidth allowed for well-performing paths (non-inclusive)", requirementMinBandwidth);
  cmd.AddValue("rttTimeTableMaxDuration", "Maximum waiting time for data packets (rtt calculation)", rttTimeTableMaxDuration);
  cmd.AddValue("topologyFile", "Path to the topology file", topologyFile);
  cmd.Parse(argc, argv);

  // Set forwarding strategy parameters
  ParameterConfiguration::getInstance()->APP_SUFFIX = appSuffix;
  ParameterConfiguration::getInstance()->PROBE_SUFFIX = probeSuffix;
  ParameterConfiguration::getInstance()->setParameter("PREFIX_OFFSET", prefixOffset, prefixA);
  ParameterConfiguration::getInstance()->setParameter("TAINTING_ENABLED", taintingEnabled, prefixA);
  ParameterConfiguration::getInstance()->setParameter("MIN_NUM_OF_FACES_FOR_TAINTING", minNumOfFacesForTainting, prefixA);
  ParameterConfiguration::getInstance()->setParameter("MAX_TAINTED_PROBES_PERCENTAGE", maxTaintedProbesPercentage, prefixA);
  ParameterConfiguration::getInstance()->setParameter("REQUIREMENT_MAXDELAY", requirementMaxDelay, prefixA);
  ParameterConfiguration::getInstance()->setParameter("REQUIREMENT_MAXLOSS", requirementMaxLoss, prefixA);
  ParameterConfiguration::getInstance()->setParameter("REQUIREMENT_MINBANDWIDTH", requirementMinBandwidth, prefixA);
  ParameterConfiguration::getInstance()->setParameter("RTT_TIME_TABLE_MAX_DURATION", rttTimeTableMaxDuration, prefixA);

  ParameterConfiguration::getInstance()->setParameter("PREFIX_OFFSET", prefixOffset, prefixB);
  ParameterConfiguration::getInstance()->setParameter("TAINTING_ENABLED", taintingEnabled, prefixB);
  ParameterConfiguration::getInstance()->setParameter("MIN_NUM_OF_FACES_FOR_TAINTING", minNumOfFacesForTainting, prefixB);
  ParameterConfiguration::getInstance()->setParameter("MAX_TAINTED_PROBES_PERCENTAGE", maxTaintedProbesPercentage, prefixB);
  ParameterConfiguration::getInstance()->setParameter("REQUIREMENT_MAXDELAY", requirementMaxDelay, prefixB);
  ParameterConfiguration::getInstance()->setParameter("REQUIREMENT_MAXLOSS", requirementMaxLoss, prefixB);
  ParameterConfiguration::getInstance()->setParameter("REQUIREMENT_MINBANDWIDTH", requirementMinBandwidth, prefixB);
  ParameterConfiguration::getInstance()->setParameter("RTT_TIME_TABLE_MAX_DURATION", rttTimeTableMaxDuration, prefixB);


  // RNG handling
  Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable> ();
  int randomNodeNumber1 = rng->GetInteger(0, 11); // (0, number of nodes - 1)
  int randomNodeNumber2 = rng->GetInteger(0, 11); // (0, number of nodes - 1)
  int randomLinkNumbers[std::stoi(linkErrorParam)];
  double linkFailureDurations[std::stoi(linkErrorParam)];
  
  for (int i = 0; i < std::stoi(linkErrorParam); ++i)
  {
    randomLinkNumbers[i] = rng->GetInteger(0, 14); // (0, number of links - 1)
  }

  for (int i = 0; i < std::stoi(linkErrorParam); ++i)
  {
    linkFailureDurations[i] = rng->GetInteger( std::stoi(linkFailureDuration) * (1.0 - std::stod(linkFailureDurationVariation)),
                                               std::stoi(linkFailureDuration) * (1.0 + std::stod(linkFailureDurationVariation))); // (e.g. (200 * 0.8, 200 *1.2))
  }

  // Print parameters
  std::cout << std::endl;
  std::cout << "Variables" << std::endl;
  std::cout << "randomNodeNumber1: " << randomNodeNumber1 << std::endl;
  std::cout << "randomNodeNumber2: " << randomNodeNumber2 << std::endl;
  std::cout << "Parameters" << std::endl;
  std::cout << "logDir: " << logDir << std::endl;
  std::cout << "forwardingStrategy: " << forwardingStrategy << std::endl;
  std::cout << "queue: " << queue << std::endl;
  std::cout << "approach: " << approach << std::endl;
  std::cout << "PI Refresh Frequency: " << piRefreshFrequency << std::endl;
  std::cout << "Link errors: " << linkErrorParam << std::endl;
  std::cout << "linkFailureDuration: " << linkFailureDuration << std::endl;
  std::cout << "linkFailureDurationVariation: " << linkFailureDurationVariation << std::endl;
  std::cout << "appSuffix: " << appSuffix << std::endl;
  std::cout << "probeSuffix: " << probeSuffix << std::endl;
  std::cout << "prefixOffset: " << prefixOffset << std::endl;
  std::cout << "taintingEnabled: " << taintingEnabled << std::endl;
  std::cout << "minNumOfFacesForTainting: " << minNumOfFacesForTainting << std::endl;
  std::cout << "maxTaintedProbesPercentage: " << maxTaintedProbesPercentage << std::endl;
  std::cout << "requirementMaxDelay: " << requirementMaxDelay << std::endl;
  std::cout << "requirementMaxLoss: " << requirementMaxLoss << std::endl;
  std::cout << "requirementMinBandwidth: " << requirementMinBandwidth << std::endl;
  std::cout << "rttTimeTableMaxDuration: " << rttTimeTableMaxDuration << std::endl;
  std::cout << std::endl;

  // Read topology
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName(topologyFile);
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Find consumer and producer nodes
  Ptr<Node> nodeA; // consumerA & producerB
  Ptr<Node> nodeB; // consumerB & producerA

  switch(randomNodeNumber1) 
  {
    case 0 :  nodeA = Names::Find<Node>("ATLA-M5");break; 
    case 1 :  nodeA = Names::Find<Node>("ATLAng"); break;
    case 2 :  nodeA = Names::Find<Node>("CHINng"); break;
    case 3 :  nodeA = Names::Find<Node>("DNVRng"); break;
    case 4 :  nodeA = Names::Find<Node>("HSTNng"); break;
    case 5 :  nodeA = Names::Find<Node>("IPLSng"); break;
    case 6 :  nodeA = Names::Find<Node>("KSCYng"); break;
    case 7 :  nodeA = Names::Find<Node>("LOSAng"); break;
    case 8 :  nodeA = Names::Find<Node>("NYCMng"); break;
    case 9 :  nodeA = Names::Find<Node>("SNVAng"); break;
    case 10:  nodeA = Names::Find<Node>("STTLng"); break;
    case 11:  nodeA = Names::Find<Node>("WASHng"); break;
    default:  nodeA = Names::Find<Node>("STTLng"); break;
  }

  switch(randomNodeNumber2) 
  {
    case 0 :  nodeB = Names::Find<Node>("ATLA-M5");break; 
    case 1 :  nodeB = Names::Find<Node>("ATLAng"); break;
    case 2 :  nodeB = Names::Find<Node>("CHINng"); break;
    case 3 :  nodeB = Names::Find<Node>("DNVRng"); break;
    case 4 :  nodeB = Names::Find<Node>("HSTNng"); break;
    case 5 :  nodeB = Names::Find<Node>("IPLSng"); break;
    case 6 :  nodeB = Names::Find<Node>("KSCYng"); break;
    case 7 :  nodeB = Names::Find<Node>("LOSAng"); break;
    case 8 :  nodeB = Names::Find<Node>("NYCMng"); break;
    case 9 :  nodeB = Names::Find<Node>("SNVAng"); break;
    case 10:  nodeB = Names::Find<Node>("STTLng"); break;
    case 11:  nodeB = Names::Find<Node>("WASHng"); break;
    default:  nodeB = Names::Find<Node>("WASHng"); break;
  }

  // Set forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/" + forwardingStrategy);

  // Prepare applications 
  ndn::AppHelper consumerHelper("ns3::ndn::PushConsumer");
  consumerHelper.SetAttribute("LifeTime", StringValue("5s"));
  consumerHelper.SetAttribute("PIRefreshInterval", StringValue(piRefreshFrequency));
  consumerHelper.SetAttribute("ProbeFrequency", StringValue("30"));

  ndn::AppHelper pushProducerHelper("ns3::ndn::PushProducer");
  pushProducerHelper.SetAttribute("Frequency", StringValue("100")); 
  pushProducerHelper.SetAttribute("PayloadSize", StringValue("82"));

  ndn::AppHelper ProbeProducerHelper("ns3::ndn::ProbeDataProducer");
  ProbeProducerHelper.SetAttribute("PayloadSize", StringValue("1")); //bytes per probe data packet

  consumerHelper.SetPrefix(prefixA_App);
  pushProducerHelper.SetPrefix(prefixA_App);
  ProbeProducerHelper.SetPrefix(prefixA_Probe);
  consumerHelper.Install(nodeA);
  pushProducerHelper.Install(nodeB);
  ProbeProducerHelper.Install(nodeB);

  consumerHelper.SetPrefix(prefixB_App);
  pushProducerHelper.SetPrefix(prefixB_App);
  ProbeProducerHelper.SetPrefix(prefixB_Probe);
  consumerHelper.Install(nodeB);
  pushProducerHelper.Install(nodeA);
  ProbeProducerHelper.Install(nodeA);

  // Add prefix origins to ndn::GlobalRouter
  ndnGlobalRoutingHelper.AddOrigins(prefixA_App, nodeB);
  ndnGlobalRoutingHelper.AddOrigins(prefixA_Probe, nodeB);
  ndnGlobalRoutingHelper.AddOrigins(prefixB_App, nodeA);
  ndnGlobalRoutingHelper.AddOrigins(prefixB_Probe, nodeA);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();

  // Simulate link failures
  uint32_t timeBetweenFailures = (simTime/1000) / std::stoi(linkErrorParam); // (Simtime in seconds) / (# of failures)

  std::cout << "start time              connection                duration" << std::endl;
  for (int i = 0; i < std::stoi(linkErrorParam); ++i)
  {
    Ptr<Node> linkNode1;
    Ptr<Node> linkNode2;

    switch(randomLinkNumbers[i]) 
    {
      case 0 : linkNode1 = Names::Find<Node>("ATLAng"); linkNode2 = Names::Find<Node>("ATLA-M5"); break;
      case 1 : linkNode1 = Names::Find<Node>("ATLAng"); linkNode2 = Names::Find<Node>("HSTNng"); break;
      case 2 : linkNode1 = Names::Find<Node>("ATLAng"); linkNode2 = Names::Find<Node>("IPLSng"); break;
      case 3 : linkNode1 = Names::Find<Node>("ATLAng"); linkNode2 = Names::Find<Node>("WASHng"); break;
      case 4 : linkNode1 = Names::Find<Node>("CHINng"); linkNode2 = Names::Find<Node>("IPLSng"); break;
      case 5 : linkNode1 = Names::Find<Node>("CHINng"); linkNode2 = Names::Find<Node>("NYCMng"); break;
      case 6 : linkNode1 = Names::Find<Node>("DNVRng"); linkNode2 = Names::Find<Node>("KSCYng"); break;
      case 7 : linkNode1 = Names::Find<Node>("DNVRng"); linkNode2 = Names::Find<Node>("SNVAng"); break;
      case 8 : linkNode1 = Names::Find<Node>("DNVRng"); linkNode2 = Names::Find<Node>("STTLng"); break;
      case 9 : linkNode1 = Names::Find<Node>("HSTNng"); linkNode2 = Names::Find<Node>("KSCYng"); break;
      case 10: linkNode1 = Names::Find<Node>("HSTNng"); linkNode2 = Names::Find<Node>("LOSAng"); break;
      case 11: linkNode1 = Names::Find<Node>("IPLSng"); linkNode2 = Names::Find<Node>("KSCYng"); break;
      case 12: linkNode1 = Names::Find<Node>("LOSAng"); linkNode2 = Names::Find<Node>("SNVAng"); break;
      case 13: linkNode1 = Names::Find<Node>("NYCMng"); linkNode2 = Names::Find<Node>("WASHng"); break;
      case 14: linkNode1 = Names::Find<Node>("SNVAng"); linkNode2 = Names::Find<Node>("STTLng"); break;
      default: linkNode1 = Names::Find<Node>("DNVRng"); linkNode2 = Names::Find<Node>("KSCYng"); break;
    }
    // Link down
    Simulator::Schedule(Seconds(0.0 + i * timeBetweenFailures), 
      ndn::LinkControlHelper::FailLink, linkNode1, linkNode2);
    // Link up
    Simulator::Schedule(Seconds(0.0 + i * timeBetweenFailures + linkFailureDurations[i]), 
      ndn::LinkControlHelper::UpLink, linkNode1, linkNode2);

    std::cout << "Linkfailure_" << i << ": " << (0.0 + i * timeBetweenFailures) << "      Node_" 
              << linkNode1->GetId() << " --x-- Node_" << linkNode2->GetId() << "      " << linkFailureDurations[i] << " [" 
              << (std::stoi(linkFailureDuration) * (1.0 - std::stod(linkFailureDurationVariation))) << ", " 
              << (std::stoi(linkFailureDuration) * (1.0 + std::stod(linkFailureDurationVariation))) << "]" << std::endl;  
  }
  std::cout << std::endl;

  // Tracer
  NodeContainer pushParticipants;
  pushParticipants.Add(nodeA);
  pushParticipants.Add(nodeB);
  ns3::ndn::PushTracer::Install(pushParticipants, std::string(logDir + "push-trace.txt"));
  ns3::ndn::L3PacketTracer::InstallAll(std::string(logDir + "packet-trace.txt"));


  Simulator::Stop(MilliSeconds(simTime));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
