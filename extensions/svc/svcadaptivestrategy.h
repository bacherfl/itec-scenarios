#ifndef SVCADAPTIVESTRATEGY_H
#define SVCADAPTIVESTRATEGY_H

#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/network-module.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-fib.h"
#include "ns3-dev/ns3/ndn-fib-entry.h"
#include "ns3-dev/ns3/ndn-pit-entry.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"

#include <boost/ref.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>


#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"

#include "../utils/ndntracer.h"
#include "svcleveltag.h"
#include "svcbitratetag.h"

#include <stdio.h>


namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SVCAdaptiveStrategy
template<class Parent>
class SVCAdaptiveStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SVCAdaptiveStrategy () {tracer = NULL;}

  virtual void AddFace(Ptr<Face> face);
  virtual void RemoveFace(Ptr<Face> face);

  virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);

protected:

  virtual uint64_t getResidualBandwidth(Ptr<Face> face, Ptr< Interest > interest);
  virtual uint64_t getResidualBandwidth(Ptr<Face> face);
  virtual uint64_t residualBandwidthOfUplinkFaces(Ptr< Interest > interest);

  static LogComponent g_log;
private:
  utils::NDNTracer* tracer;
};

template<class Parent>
LogComponent SVCAdaptiveStrategy<Parent>::g_log = LogComponent (SVCAdaptiveStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId SVCAdaptiveStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SVCAdaptiveStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SVCAdaptiveStrategy> ();
  return tid;
}

template<class Parent>
std::string SVCAdaptiveStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".SVCAdaptiveStrategy";
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::AddFace (Ptr<Face> face)
{
  if(tracer == NULL)
    tracer = new utils::NDNTracer(this);

  tracer->addFace (face);
  super::AddFace(face);
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::RemoveFace (Ptr<Face> face)
{
  if(tracer != NULL)
    tracer->removeFace(face);
  super::AddFace(face);
}

template<class Parent>
uint64_t SVCAdaptiveStrategy<Parent>::getResidualBandwidth(Ptr<Face> face)
{
  // Get Device Bitrate
  Ptr<PointToPointNetDevice> d = face->GetNode()->GetDevice(0)->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  d->GetAttribute("DataRate", dv);
  DataRate rate = dv.Get();

  uint64_t availableBitrate = rate.GetBitRate();
  uint64_t avgTrafficOnFace = tracer->getAvgTotalTrafficBits (face);

  uint64_t residualBandwidth = 0;

  if(availableBitrate > avgTrafficOnFace)
    residualBandwidth = availableBitrate - avgTrafficOnFace;

  return residualBandwidth;
}

template<class Parent>
uint64_t SVCAdaptiveStrategy<Parent>::getResidualBandwidth(Ptr<Face> face, Ptr< Interest > interest)
{

  uint64_t residualBandwidthOnFace = getResidualBandwidth (face);

  Ptr<Packet> packet = Wire::FromInterest(interest);
  SVCBitrateTag bitrateTag;

  uint64_t residualBandwidth = 0;
  bool tagExists = packet->PeekPacketTag (bitrateTag);

  if(tagExists)
  {
    uint64_t requiredBandwidth = bitrateTag.Get ();

    if(residualBandwidthOnFace > requiredBandwidth)
      residualBandwidth = residualBandwidthOnFace - requiredBandwidth;
  }
  return residualBandwidth;
}

template<class Parent>
uint64_t SVCAdaptiveStrategy<Parent>::residualBandwidthOfUplinkFaces(Ptr<Interest> interest)
{

  uint64_t residualBandwidth = 0;
  Ptr<fib::Entry > e = NULL;

  std::string i_name = interest->GetName ().toUri();
  for (Ptr<fib::Entry > entry = SVCAdaptiveStrategy<Parent>::m_fib->Begin ();
       entry != SVCAdaptiveStrategy<Parent>::m_fib->End ();
       entry = SVCAdaptiveStrategy<Parent>::m_fib->Next(entry))
  {
    //fprintf(stderr, "i_name %s\n", i_name.c_str ());
    //fprintf(stderr, "ntry->GetPrefix() %s\n", entry->GetPrefix().toUri().c_str());
    if(i_name.find(entry->GetPrefix().toUri().c_str()) == 0)
    {
      e = entry;
      break;
    }
  }

  if(e != NULL)
  {
    //add up the resiudal bandwidth of all uplink faces
    //fprintf(stderr, "Fibentry found = %s \n", e->GetPrefix ().toUri().c_str());

    for (ndn::fib::FaceMetricContainer::type::index<ndn::fib::i_face>::type::iterator metric =
           (*e).m_faces.get<ndn::fib::i_face> ().begin ();
         metric != (*e).m_faces.get<ndn::fib::i_face> ().end ();
         metric++)
      {
        residualBandwidth += getResidualBandwidth(metric->GetFace());
      }
  }

  Ptr<Packet> packet = Wire::FromInterest(interest);
  SVCBitrateTag bitrateTag;

  bool tagExists = packet->PeekPacketTag (bitrateTag);

  if(tagExists)
  {
    uint64_t requiredBandwidth = bitrateTag.Get ();

    if(residualBandwidth > requiredBandwidth)
      residualBandwidth -= requiredBandwidth;
    else
      residualBandwidth = 0;

  }

  return residualBandwidth;
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{

  //check if first chunk of segment
  std::string name = interest->GetName ().toUri();
  if(name.size () > 6)
    name = name.substr (name.size () - 7);

  if(name.compare ("chunk_0") != 0)
  {
    super::OnInterest(inface,interest);
    return;
  }

  //fprintf(stderr, "name %s\n", name.c_str ());

  Ptr<Packet> packet = Wire::FromInterest(interest);
  SVCLevelTag levelTag;

  //check if enchancment layer
  bool tagExists = packet->PeekPacketTag(levelTag);

  if (tagExists && levelTag.Get () > 0) // layer is an enchancment layer
  {
    //check if enough bandwidth is available for uplink / downlink transmission.

    if(getResidualBandwidth(inface, interest) == 0 && residualBandwidthOfUplinkFaces(interest) == 0)
    {
      NS_LOG_UNCOND("Strategy: Dropping Interest " << interest->GetName ().toUri());

      Ptr<Interest> nack = Create<Interest> (*interest);
      nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

      levelTag.Set (-1); // means packet dropped on purpose
      nack->GetPayload ()->AddPacketTag (levelTag);

      inface->SendInterest (nack);
      SVCAdaptiveStrategy<Parent>::m_outNacks (nack, inface);
      return;
    }
  }

  super::OnInterest(inface,interest);
}

} // namespace fw
} // namespace ndn
} // namespace ns3
#endif // SVCADAPTIVESTRATEGY_H
