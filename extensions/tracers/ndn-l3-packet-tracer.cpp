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

#include "ndn-l3-packet-tracer.hpp"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/callback.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node-list.h"

#include "daemon/table/pit-entry.hpp"

#include <fstream>
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.L3PacketTracer");

namespace ns3 {
namespace ndn {

static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<L3PacketTracer>>>>
  g_tracers;

void
L3PacketTracer::Destroy()
{
  g_tracers.clear();
}

void
L3PacketTracer::InstallAll(const std::string& file)
{
  std::list<Ptr<L3PacketTracer>> tracers;
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
    Ptr<L3PacketTracer> trace = Install(*node, outputStream);
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
L3PacketTracer::Install(const NodeContainer& nodes, const std::string& file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<L3PacketTracer>> tracers;
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
    Ptr<L3PacketTracer> trace = Install(*node, outputStream);
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
L3PacketTracer::Install(Ptr<Node> node, const std::string& file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<L3PacketTracer>> tracers;
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

  Ptr<L3PacketTracer> trace = Install(node, outputStream);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<L3PacketTracer>
L3PacketTracer::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream)
{
  NS_LOG_DEBUG("Node: " << node->GetId());

  Ptr<L3PacketTracer> trace = Create<L3PacketTracer>(outputStream, node);

  return trace;
}

L3PacketTracer::L3PacketTracer(shared_ptr<std::ostream> os, Ptr<Node> node)
  : L3Tracer(node)
  , m_os(os)
{
}

L3PacketTracer::L3PacketTracer(shared_ptr<std::ostream> os, const std::string& node)
  : L3Tracer(node)
  , m_os(os)
{
}

L3PacketTracer::~L3PacketTracer()
{
  m_printEvent.Cancel();
}

void
L3PacketTracer::PrintHeader(std::ostream& os) const
{
  os << "Time"
     << "\t"

     << "Node"
     << "\t"
     << "FaceId"
     << "\t"
     << "FaceDescr"
     << "\t"

     << "Type"
     << "\t"
     << "Name"
     << "\t"
     << "Kilobytes";
}

void
L3PacketTracer::Print(std::ostream& os) const
{
  
}

const double alpha = 0.8;


void
L3PacketTracer::printLog(uint32_t faceId, std::string faceDescr, std::string type, std::string name, uint32_t kilobytes)
{
  Time time = Simulator::Now();                                                                                   
  *m_os << time.ToDouble(Time::S) << "\t" << m_node << "\t";           
  *m_os << faceId << "\t" << faceDescr << "\t";                        
  *m_os << type << "\t" << name << "\t" << kilobytes << "\n";           
}

void
L3PacketTracer::OutInterests(const Interest& interest, const Face& face)
{
  this->printLog(face.getId(), face.getLocalUri().toString(), "OutInterest", interest.getName().toUri(), (uint32_t)interest.wireEncode().size());
}

void
L3PacketTracer::InInterests(const Interest& interest, const Face& face)
{
  this->printLog(face.getId(), face.getLocalUri().toString(), "InInterest", interest.getName().toUri(), (uint32_t)interest.wireEncode().size());
}

void
L3PacketTracer::OutData(const Data& data, const Face& face)
{
  this->printLog(face.getId(), face.getLocalUri().toString(), "OutData", data.getName().toUri(), (uint32_t)data.wireEncode().size());
}

void
L3PacketTracer::InData(const Data& data, const Face& face)
{
  this->printLog(face.getId(), face.getLocalUri().toString(), "InData", data.getName().toUri(), (uint32_t)data.wireEncode().size());
}

void
L3PacketTracer::OutNack(const lp::Nack& nack, const Face& face)
{
  // todo: Implement Nack method
}

void
L3PacketTracer::InNack(const lp::Nack& nack, const Face& face)
{
  // todo: Implement Nack method
}

void
L3PacketTracer::SatisfiedInterests(const nfd::pit::Entry& entry, const Face&, const Data&)
{
}

void
L3PacketTracer::TimedOutInterests(const nfd::pit::Entry& entry)
{
}

} // namespace ndn
} // namespace ns3
