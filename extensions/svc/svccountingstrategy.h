#ifndef SVCCOUNTINGSTRATEGY_H
#define SVCCOUNTINGSTRATEGY_H


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

#include "ns3-dev/ns3/random-variable.h"

#include "svcleveltag.h"
#include "svcbitratetag.h"
#include "facepacketstatistic.h"

#include "../utils/deadlinetag.h"
// include idownloader interface for PACKET_SIZE and OVERHEAD
#include "../utils/idownloader.h"

#include "../droppingPolicy/smoothlevelstatistics.h"
#include "../droppingPolicy/packetbaseddroppingpolicy.h"

// include boost libraries for map
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>


#include <stdio.h>


#define UPDATE_ALPHA 0.2
#define NUM_LEVELS   3
#define RESET_STATISTICS_TIMER 1.0

// set the default content level (default: 0) - set to 1 if you want to punish non svc content
#define DEFAULT_CONTENT_LEVEL 0


namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SVCCountingStrategy
template<class Parent>
class SVCCountingStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SVCCountingStrategy () : super() {
    fprintf(stderr, "In constructor...\n");
    // init the uniform random variable
    randomNumber = UniformVariable(0,1);

    // init amount of levels
    //stats.CreateLevels(NUM_LEVELS);

    // schedule first reset event
    ResetStatisticsTimer = Simulator::Schedule(Seconds(RESET_STATISTICS_TIMER), &SVCCountingStrategy::resetLevelCount, this);
  }

  virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);
  virtual void AddFace(Ptr< Face> face);
  virtual void RemoveFace(Ptr< Face > face);
  virtual bool CanSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace,
                                   Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);
  virtual void DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest,
                                            Ptr<pit::Entry> pitEntry);


  static uint64_t getPhysicalBitrate(Ptr<Face> face);

protected:
  void resetLevelCount();
  bool HasEnoughResourcesToSend(Ptr< Face > face, Ptr< const Interest > interest);

  UniformVariable randomNumber;

  /* vector to store all faces */
  std::vector<Ptr<ndn::Face> > faces;

  /* map for storing stats for all faces */
  typedef boost::unordered_map
    <
      int/*faceID*/,
      svc::FacePacketStatistic*
    > FacePacketStatisticMap;

  FacePacketStatisticMap map;


  static LogComponent g_log;

  EventId ResetStatisticsTimer;
};

template<class Parent>
LogComponent SVCCountingStrategy<Parent>::g_log = LogComponent (SVCCountingStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId SVCCountingStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SVCCountingStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SVCCountingStrategy> ();
  return tid;
}

template<class Parent>
std::string SVCCountingStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".SVCCountingStrategy";
}


template<class Parent>
uint64_t SVCCountingStrategy<Parent>::getPhysicalBitrate(Ptr<Face> face)
{
  // Get Device Bitrate
  Ptr<PointToPointNetDevice> nd1 =
      face->GetNode()->GetDevice(0)->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  nd1->GetAttribute("DataRate", dv);
  DataRate d = dv.Get();
  return d.GetBitRate();
}


template<class Parent>
void SVCCountingStrategy<Parent>::resetLevelCount() {
  for(std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    // get face pointer
    Ptr<Face> face = *it;

    // calculate max_packets and metric
    uint64_t bitrate = getPhysicalBitrate(face);
    int max_packets = bitrate / ( MAX_PACKET_PAYLOAD + PACKET_OVERHEAD ) / 8;

    // just in case...
    max_packets = max_packets * 0.85;

    double metric = 0.0;

    // get last packets
    unsigned int last_packets_ps = this->map[face->GetId ()]->GetPacketsPerTime ();

    // calculate metric only if last_packets > max_packets, else metric stays 0.0
    if (last_packets_ps > max_packets)
      metric = ((double)(last_packets_ps-max_packets)) / (double)(last_packets_ps);

    // refresh statistics, update the policy (= feed policy and then reset stats )
    this->map[face->GetId ()]->UpdatePolicy (metric);

    // reset packets per second
    this->map[face->GetId ()]->SetPacketsPerTime (0);
  }


  // schedule next reset event
  ResetStatisticsTimer = Simulator::Schedule(Seconds(RESET_STATISTICS_TIMER), &SVCCountingStrategy::resetLevelCount, this);
}


/* add face */
template<class Parent>
void SVCCountingStrategy<Parent>::AddFace (Ptr<Face> face)
{
  // add face to our map
  map[face->GetId ()] = new svc::FacePacketStatistic();
  // tell the stats how many levels we will have
  map[face->GetId ()]->getStats()->CreateLevels(NUM_LEVELS);

  // add face to faces vector
  faces.push_back (face);

  super::AddFace(face);
}


/* remove face */
template<class Parent>
void SVCCountingStrategy<Parent>::RemoveFace (Ptr<Face> face)
{
  // first: get pointer of the Statistics object
  svc::FacePacketStatistic* p = map[face->GetId ()];

  // second: erase face from map
  map.erase (face->GetId ());

  // third: if not already happened, delete the Statistics object
  if (p != NULL)
    delete p;


  // remove face from faces vector
  for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    if (face== (*it))//todo
    {
      faces.erase (it);
      break;
    }
  }

  super::RemoveFace (face);
}

template<class Parent>
bool SVCCountingStrategy<Parent>::HasEnoughResourcesToSend
    ( Ptr< Face > face, Ptr< const Interest > interest )
{
  // get the actual packet so we can access tags
  Ptr<Packet> packet = Wire::FromInterest (interest);

  // extract level tag from packet
  SVCLevelTag levelTag;
  bool svcLevelTagExists = packet->PeekPacketTag (levelTag);

  int level = DEFAULT_CONTENT_LEVEL;
  if (svcLevelTagExists)
  {
    level = levelTag.Get ();
  }
  if (level == 16)
    level = 1;
  else if (level == 32)
    level = 2;
  else
    level = 0;



  /* DeadlineTag deadlineTag;

  uint64_t deadline = 0;

  bool deadlineTagExists = packet->PeekPacketTag (deadlineTag);
  if (deadlineTagExists)
  {
    //fprintf(stderr, "deadline tag found!\n");
    deadline = deadlineTag.Get ();
  }

  if (deadline == 0)
  {
    deadline = 1000;
  } */


  // increase level counter for that face
  this->map[face->GetId ()]->IncreasePackets (level);

  // check if RandomNumber(0,1) < DropProbability
  // if yes --> drop (=  NOT CanSendOutInterest)
  double dropProbability = this->map[face->GetId ()]->GetDropProbability (level);

  return ( randomNumber.GetValue() >= dropProbability );
}

template<class Parent>
bool SVCCountingStrategy<Parent>::CanSendOutInterest (
    Ptr< Face > inFace, Ptr< Face > outFace,
    Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  // if we can not send out an interest, return false for this face
  if (!HasEnoughResourcesToSend(outFace, interest))
    return false;
  // else: let parent class decide

  return super::CanSendOutInterest(inFace, outFace, interest, pitEntry);
}


template<class Parent>
void SVCCountingStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{
  SVCLevelTag levelTag;

  // lookup pit entry for interest (if exists)
  Ptr<pit::Entry> pitEntry = SVCCountingStrategy<Parent>::m_pit->Lookup (*interest);

  // check if duplicate interest first
  bool isDuplicate = false;
  if (pitEntry != 0)
  {
    isDuplicate = pitEntry->IsNonceSeen (interest->GetNonce () );
  }

  // check if duplicate interest first
  if (! isDuplicate)
  {
    // check if interest can be returned using the inface
    if (! HasEnoughResourcesToSend (inface, interest) )
    {
      // DROP
      // NS_LOG_UNCOND("Strategy: Dropping Interest " << interest->GetName ().toUri());

      Ptr<Interest> nack = Create<Interest> (*interest);
      nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

      levelTag.Set (-1); // means packet dropped on purpose
      nack->GetPayload ()->AddPacketTag (levelTag);


      inface->SendInterest (nack);
      SVCCountingStrategy<Parent>::m_outNacks (nack, inface);
      // nack sent - we dont need anything else --> return
      return;
    }
  }

  // let parent continue with the interest
  super::OnInterest(inface,interest);
}


template<class Parent>
void SVCCountingStrategy<Parent>::DidExhaustForwardingOptions
            (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  // create the nack packet
  SVCLevelTag levelTag;
  Ptr<Interest> nack = Create<Interest> (*interest);
  nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

  levelTag.Set (-1); // means packet dropped on purpose
  nack->GetPayload ()->AddPacketTag (levelTag);

  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
  {
    NS_LOG_DEBUG ("Send NACK for " << boost::cref (nack->GetName ()) << " to " << boost::cref (*incoming.m_face));
    incoming.m_face->SendInterest (nack);
    SVCCountingStrategy<Parent>::m_outNacks (nack, incoming.m_face);
  }

  pitEntry->ClearOutgoing (); // to force erasure of the record

  // do not call super::DidExhaust... as we are sending our own Nack;
  // Call ForwardingStrategy::DidExhaust instead
  ForwardingStrategy::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif // SVCCOUNTINGSTRATEGY_H
