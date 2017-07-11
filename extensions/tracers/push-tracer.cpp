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

#include "push-tracer.hpp"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/callback.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node-list.h"

#include "utils/ndn-ns3-packet-tag.hpp"
//#include "utils/ndn-fw-hop-count-tag.hpp"

#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"

#include <fstream>
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.PushTracer");

namespace ns3 {
namespace ndn {

static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<PushTracer>>>>
  g_tracers;

void
PushTracer::Destroy()
{
  g_tracers.clear();
}

void
PushTracer::InstallAll(const std::string& file, Time averagingPeriod /* = Seconds (0.5)*/)
{
  std::list<Ptr<PushTracer>> tracers;
  shared_ptr<std::ostream> outputStream;
  if (file != "-") {
    shared_ptr<std::ofstream> os(new std::ofstream());
    os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!os->is_open()) {
      NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
      return;
    }

    outputStream = os;
  }
  else {
    outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
  }

  for (NodeList::Iterator node = NodeList::Begin(); node != NodeList::End(); node++) {
    Ptr<PushTracer> trace = Install(*node, outputStream, averagingPeriod);
    tracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

void
PushTracer::Install(const NodeContainer& nodes, const std::string& file,
                      Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<PushTracer>> tracers;
  shared_ptr<std::ostream> outputStream;
  if (file != "-") {
    shared_ptr<std::ofstream> os(new std::ofstream());
    os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!os->is_open()) {
      NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
      return;
    }

    outputStream = os;
  }
  else {
    outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
  }

  for (NodeContainer::Iterator node = nodes.Begin(); node != nodes.End(); node++) {
    Ptr<PushTracer> trace = Install(*node, outputStream, averagingPeriod);
    tracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

void
PushTracer::Install(Ptr<Node> node, const std::string& file,
                      Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<PushTracer>> tracers;
  shared_ptr<std::ostream> outputStream;
  if (file != "-") {
    shared_ptr<std::ofstream> os(new std::ofstream());
    os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!os->is_open()) {
      NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
      return;
    }

    outputStream = os;
  }
  else {
    outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
  }

  Ptr<PushTracer> trace = Install(node, outputStream, averagingPeriod);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<PushTracer>
PushTracer::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream,
                      Time averagingPeriod /* = Seconds (0.5)*/)
{
  NS_LOG_DEBUG("Node: " << node->GetId());

  Ptr<PushTracer> trace = Create<PushTracer>(outputStream, node);
  trace->SetAveragingPeriod(averagingPeriod);

  return trace;
}

PushTracer::PushTracer(shared_ptr<std::ostream> os, Ptr<Node> node)
  : L3Tracer(node)
  , m_os(os)
{
  SetAveragingPeriod(Seconds(1.0));
}

PushTracer::PushTracer(shared_ptr<std::ostream> os, const std::string& node)
  : L3Tracer(node)
  , m_os(os)
{
  SetAveragingPeriod(Seconds(1.0));
}

PushTracer::~PushTracer()
{
  m_printEvent.Cancel();
}

void
PushTracer::SetAveragingPeriod(const Time& period)
{
  m_period = period;
  m_printEvent.Cancel();
  m_printEvent = Simulator::Schedule(m_period, &PushTracer::PeriodicPrinter, this);
}

void
PushTracer::PeriodicPrinter()
{
  Print(*m_os);
  Reset();

  m_printEvent = Simulator::Schedule(m_period, &PushTracer::PeriodicPrinter, this);
}

void
PushTracer::PrintHeader(std::ostream& os) const
{
  os << "Time"
     << "\t"
     << "Node"
     << "\t"
     << "Type"
     << "\t"
     << "Name"
     << "\t"
     << "SeqNo"
     << "\t"
     << "HopCount"
     << "";
}

void
PushTracer::Reset()
{
}

void PushTracer::Print(std::ostream& os) const
{
  Time time = Simulator::Now();

  /*for(PrefixTracerMap::const_iterator pit = prefixMap.begin (); pit != prefixMap.end (); pit++)
  {
    for(FaceMap::const_iterator fit = pit->second.begin(); fit != pit->second.end(); fit++)
    {
      os  << time
          << "\t"
          << pit->first
          << "\t"

          << fit->first
          << "\t"

          << fit->second.find("OutInterests")->second
          << "\t"
          << fit->second.find("OutInterestsByte")->second
          << "\t"
          << fit->second.find("InInterests")->second
          << "\t"
          << fit->second.find("InInterestsByte")->second
          << "\t"
          << fit->second.find("OutData")->second
          << "\t"
          << fit->second.find("OutDataByte")->second
          << "\t"
          << fit->second.find("InData")->second
          << "\t"
          << fit->second.find("InDataByte")->second
          << "\t"

          << "\n";
    }
  }*/
}

void PushTracer::OutInterests(const Interest& interest, const Face& face)
{
}

void PushTracer::InInterests(const Interest& interest, const Face& face)
{
  bool voip = interest.getName().toUri().find("voip") != std::string::npos;
  if (interest.isPush() || interest.isPushRefresh() || voip) {

    int hopCount = 0;
    
    auto hopCountTag = interest.getTag<lp::HopCountTag>();
    if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
      hopCount = *hopCountTag;
      NS_LOG_DEBUG("Hop count: " << hopCount);
    }

    // Todo: Find better distinction between InInterest and OutInterest
    uint32_t seq = 0;
    Name name = interest.getName();
    *m_os << Simulator::Now().ToDouble(Time::S) << "\t"
          << m_nodePtr->GetId() << "\t"
          << (hopCount == 0 ? "OutInterests" : "InInterests") << "\t" 
          << name << "\t"
          << seq << "\t"
          << hopCount << "\t"
          << "\n";
  }
}

void PushTracer::OutData(const Data& data, const Face& face)
{
  bool voip = data.getName().toUri().find("voip") != std::string::npos;

  if (data.isPush() || voip) {
    int hopCount = 0;
    
    auto hopCountTag = data.getTag<lp::HopCountTag>();
    if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
      hopCount = *hopCountTag;
      NS_LOG_DEBUG("Hop count: " << hopCount);
    }

    uint32_t seq = data.getName().at(-1).toSequenceNumber();

    Name name = data.getName().getPrefix(data.getName().size() - 1);

    *m_os << Simulator::Now().ToDouble(Time::S) << "\t"
          << m_nodePtr->GetId() << "\t"
          << "outData" << "\t" 
          << name << "\t"
          << seq << "\t"
          << hopCount << "\t"
          << "\n";
  }
}

void PushTracer::InData(const Data& data, const Face& face)
{
  bool voip = data.getName().toUri().find("voip") != std::string::npos;

  if (data.isPush() || voip) {
    int hopCount = 0;
    
    auto hopCountTag = data.getTag<lp::HopCountTag>();
    if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
      hopCount = *hopCountTag;
      NS_LOG_DEBUG("Hop count: " << hopCount);
    }

    uint32_t seq = data.getName().at(-1).toSequenceNumber();

    Name name = data.getName().getPrefix(data.getName().size() - 1);

    *m_os << Simulator::Now().ToDouble(Time::S) << "\t"
          << m_nodePtr->GetId() << "\t"
          << (hopCount == 0 ? "send" : "receive") << "\t" 
          << name << "\t"
          << seq << "\t"
          << hopCount << "\t"
          << "\n";
  } 
}

void
PushTracer::SatisfiedInterests(const nfd::pit::Entry& entry, const Face&, const Data&)
{
}

void
PushTracer::TimedOutInterests(const nfd::pit::Entry& entry)
{
}

void
PushTracer::OutNack(const lp::Nack& nack, const Face& face)
{
}

void
PushTracer::InNack(const lp::Nack& nack, const Face& face)
{
}

// void PushTracer::checkIfFaceKnown(const Name& name, const Face& face)
// {
//   checkIfPrefixKnown (name);

//   std::string prefix = name.get (0).toUri ();
//   int fid = face.getId ();

//   if(prefixMap[prefix].find(fid) == prefixMap[prefix].end())
//   {
//     prefixMap[prefix][fid] = AttributeMap();
//     prefixMap[prefix][fid]["InInterests"]=0;
//     prefixMap[prefix][fid]["OutInterests"]=0;
//     prefixMap[prefix][fid]["InData"]=0;
//     prefixMap[prefix][fid]["OutData"]=0;

//     prefixMap[prefix][fid]["InInterestsByte"]=0;
//     prefixMap[prefix][fid]["OutInterestsByte"]=0;
//     prefixMap[prefix][fid]["InDataByte"]=0;
//     prefixMap[prefix][fid]["OutDataByte"]=0;
//   }

// }

// void PrefixTracer::checkIfPrefixKnown(const Data& data)
// {
//   checkIfPrefixKnown(data.getName ());
// }

// void PrefixTracer::checkIfPrefixKnown(const Interest& interest)
// {
//   checkIfPrefixKnown (interest.getName ());
// }

// void PrefixTracer::checkIfPrefixKnown(const Name& name)
// {
//   std::string prefix = name.get (0).toUri ();

//   if(prefixMap.find (prefix) == prefixMap.end ())
//   {
//     prefixMap[prefix] = FaceMap();
//   }
// }

} // namespace ndn
} // namespace ns3
