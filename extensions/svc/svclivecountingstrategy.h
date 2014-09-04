#ifndef SVCLIVECOUNTINGSTRATEGY_H
#define SVCLIVECOUNTINGSTRATEGY_H


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
#define RESET_STATISTICS_TIMER 1.0

#define DEFAULT_AMOUNT_LEVELS 1
// set the default content level (default: 0) - set to 1 if you want to punish non svc content
#define DEFAULT_CONTENT_LEVEL 0


namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SVCLiveCountingStrategy
template<class Parent>
class SVCLiveCountingStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();
  static std::string GetLogName ();

  SVCLiveCountingStrategy () : super()
  {
    fprintf(stderr, "SVCLiveCountingStrategy activated...\n");

    // init the uniform random variable
    randomNumber = UniformVariable(0,1);

    // init amount of levels
    //stats.CreateLevels(m_levelCount);

    // schedule first reset event
    ResetStatisticsTimer = Simulator::Schedule(Seconds(RESET_STATISTICS_TIMER), &SVCLiveCountingStrategy::resetLevelCounts, this);
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
  void resetLevelCounts();
  void resetLevelCount(Ptr<ndn::Face> face);
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

  int m_levelCount;

  static LogComponent g_log;

  Time lastResetTime;

  EventId ResetStatisticsTimer;
};

template<class Parent>
LogComponent SVCLiveCountingStrategy<Parent>::g_log = LogComponent (SVCLiveCountingStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId SVCLiveCountingStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SVCLiveCountingStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SVCLiveCountingStrategy> ()
      .template AddAttribute("LevelCount", "The amount of levels as a positive integer > 0",
                    IntegerValue(DEFAULT_AMOUNT_LEVELS),
                    MakeIntegerAccessor(&SVCLiveCountingStrategy<Parent>::m_levelCount),
                             MakeIntegerChecker<int32_t>());
  return tid;
}

template<class Parent>
std::string SVCLiveCountingStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".SVCLiveCountingStrategy";
}


template<class Parent>
uint64_t SVCLiveCountingStrategy<Parent>::getPhysicalBitrate(Ptr<Face> face)
{
  // Get Device Bitrate of that face (make sure to call face->Getid()
  Ptr<PointToPointNetDevice> nd1 =
      face->GetNode()->GetDevice(face->GetId())->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  nd1->GetAttribute("DataRate", dv);
  DataRate d = dv.Get();
  return d.GetBitRate();
}


template<class Parent>
void SVCLiveCountingStrategy<Parent>::resetLevelCount(Ptr<ndn::Face> face) {
  // calculate max_packets and metric
  uint64_t bitrate = getPhysicalBitrate(face);
  int max_packets = bitrate / ( MAX_PACKET_PAYLOAD + PACKET_OVERHEAD ) / 8;
  max_packets = max_packets * 1.0;

  // set max packets
  this->map[face->GetId()]->SetMaxPacketsPerTime(max_packets);

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


template<class Parent>
void SVCLiveCountingStrategy<Parent>::resetLevelCounts() {
  for(std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    // get face pointer
    Ptr<Face> face = *it;
    resetLevelCount(face);
  }

  lastResetTime = Simulator::Now();

  // schedule next reset event
  ResetStatisticsTimer = Simulator::Schedule(Seconds(RESET_STATISTICS_TIMER), &SVCLiveCountingStrategy::resetLevelCounts, this);
}


/* add face */
template<class Parent>
void SVCLiveCountingStrategy<Parent>::AddFace (Ptr<Face> face)
{
  // add face to our map
  map[face->GetId ()] = new svc::FacePacketStatistic();
  // tell the stats how many levels we will have
  map[face->GetId ()]->getStats()->CreateLevels(m_levelCount);

  // add face to faces vector
  faces.push_back (face);

  super::AddFace(face);
}


/* remove face */
template<class Parent>
void SVCLiveCountingStrategy<Parent>::RemoveFace (Ptr<Face> face)
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
bool SVCLiveCountingStrategy<Parent>::HasEnoughResourcesToSend
    ( Ptr< Face > face, Ptr< const Interest > interest )
{
  // check chunk number
  /*std::string fname = interest->GetName().toUri();  // get the uri from interest
  std::string chunk_nr = fname.substr(fname.find_last_of ("/chunk_")+1); // extract the chunk number remove .../chunk_X
  fname = fname.substr (0, fname.find_last_of ("/"));

  int chunk_number;
  std::stringstream(chunk_nr) >> chunk_number;*/

  // get the actual packet so we can access tags
  Ptr<Packet> packet = Wire::FromInterest (interest);

  // extract level tag from packet
  SVCLevelTag levelTag;
  bool svcLevelTagExists = packet->PeekPacketTag (levelTag);

  int level = DEFAULT_CONTENT_LEVEL;

  //TODO fix levels
  if (svcLevelTagExists)
  {
    level = levelTag.Get ();

    //todo just for spaital bunny_2s content.

    if(level == 10)
      level = 1;
    else if(level == 20)
      level = 2;

  }

  // increase level counter for that face
  this->map[face->GetId ()]->IncreasePackets (level);

  // calculate time since last reset
  int diff = (int) (Simulator::Now().GetMilliSeconds () - lastResetTime.GetMilliSeconds ());
  // should be less than 1000


  int packets = this->map[face->GetId()]->GetPacketsPerTime();

  // calculate new metric
  // max_packets = per second
  // now max_packets per "diff" time
  int new_max_packets = this->map[face->GetId()]->GetMaxPacketsPerTime();

  // dont do anything in the first 100 ms because stats are going to be "bad"
  if (diff > 100)
  {
    new_max_packets = ( (double)(new_max_packets * diff) ) / ((double)1000);
    if (packets > new_max_packets)
    {
      double metric = ((double)(packets-new_max_packets)) / (double)(packets);
      //fprintf(stderr, "diff=%d, p=%d, newp=%d, metric=%f\n", diff, packets, new_max_packets, metric);

      this->map[face->GetId ()]->UpdatePolicy (metric);
    }
  }

  // check if RandomNumber(0,1) < DropProbability
  // if yes --> drop (=  NOT CanSendOutInterest)
  double dropProbability = this->map[face->GetId ()]->GetDropProbability (level);

  if(level == 0)
    return true;

  /*//check if level 0
  if (level == 0 && !(randomNumber.GetValue() >= dropProbability) )
  {
    //extract the video

    std::string video = interest->GetName().toUri();
    video = video.substr (0, video.find_last_of ("/"));
    video = video.substr (0, video.find_last_of ("/"));
    video = video.substr (video.find_last_of ("/")+1);

    //fprintf(stderr, "video = %s\n", video.c_str ());
    //fprintf(stderr, "Pitsize = %d \n",SVCLiveCountingStrategy<Parent>::m_pit->GetSize());

    Ptr<pit::Entry> entry = SVCLiveCountingStrategy<Parent>::m_pit->Begin();

    std::map<std::string, int> different_streams;

    double ratio = 0;
    while (entry != SVCLiveCountingStrategy<Parent>::m_pit->End())
    {
      std::string pit_entry_name = entry->GetInterest ()->GetName ().toUri ();
      pit_entry_name = pit_entry_name.substr (0, pit_entry_name.find_last_of ("/"));
      pit_entry_name = pit_entry_name.substr (0, pit_entry_name.find_last_of ("/"));
      pit_entry_name = pit_entry_name.substr (pit_entry_name.find_last_of ("/")+1);

      if(pit_entry_name.compare (video) == 0)
        ratio++;

      different_streams[pit_entry_name] = 0;

      entry = SVCLiveCountingStrategy<Parent>::m_pit->Next(entry);
    }
    ratio /= SVCLiveCountingStrategy<Parent>::m_pit->GetSize();

    //fprintf(stderr, "Ratio for %s = %f\n", video.c_str (), ratio);
    //fprintf(stderr, "Different streams = %d\n", different_streams.size ());

    double allowed_ratio = 1.0 / (double) different_streams.size ();

    if(ratio >=  allowed_ratio * 2)
      return false;

    return true;
  }*/

  return ( randomNumber.GetValue() >= dropProbability );
}

template<class Parent>
bool SVCLiveCountingStrategy<Parent>::CanSendOutInterest (
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
void SVCLiveCountingStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{
  SVCLevelTag levelTag;

  // lookup pit entry for interest (if exists)
  Ptr<pit::Entry> pitEntry = SVCLiveCountingStrategy<Parent>::m_pit->Lookup (*interest);

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
      //NS_LOG_UNCOND("Strategy::OnInterest Dropping Interest " << interest->GetName ().toUri());
      Ptr<Interest> nack = Create<Interest> (*interest);
      nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

      levelTag.Set (-1); // means packet dropped on purpose
      nack->GetPayload ()->AddPacketTag (levelTag);

      inface->SendInterest (nack);
      SVCLiveCountingStrategy<Parent>::m_outNacks (nack, inface);
      // nack sent - we dont need anything else --> return
      return;
    }
  }

  // let parent continue with the interest
  super::OnInterest(inface,interest);
}


template<class Parent>
void SVCLiveCountingStrategy<Parent>::DidExhaustForwardingOptions
            (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  // create the nack packet
  SVCLevelTag levelTag;
  Ptr<Interest> nack = Create<Interest> (*interest);
  nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

  levelTag.Set (-1); // means packet dropped on purpose
  //NS_LOG_UNCOND("Strategy::DidExhaustForwardingOptions Dropping Interest " << interest->GetName ().toUri());
  nack->GetPayload ()->AddPacketTag (levelTag);

  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
  {
    //NS_LOG_UNCOND ("Send NACK for " << boost::cref (nack->GetName ()) << " to " << boost::cref (*incoming.m_face));
    incoming.m_face->SendInterest (nack);
    SVCLiveCountingStrategy<Parent>::m_outNacks (nack, incoming.m_face);
  }

  pitEntry->ClearOutgoing (); // to force erasure of the record

  // do not call super::DidExhaust... as we are sending our own Nack;
  // Call ForwardingStrategy::DidExhaust instead
  ForwardingStrategy::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif // SVCLIVECOUNTINGSTRATEGY_H
