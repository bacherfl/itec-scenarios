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
#include "../utils/deadlinetag.h"


#include <stdio.h>


#define UPDATE_ALPHA 0.2

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
    levelCount.reserve(3);
    levelProb.reserve(3);
    dropProb.reserve(3);
    levelCount[0] = levelCount[1] = levelCount[2] = 0;
    levelProb[0] = levelProb[1] = levelProb[2] = 0.0;
    dropProb[0] = dropProb[1] = dropProb[2] = 0.0;
    resetLevelCount();
  }

  virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);

protected:
  std::vector<unsigned int> levelCount;
  std::vector<double> levelProb;
  std::vector<double> dropProb;

  unsigned int last_packets_per_second;

  void resetLevelCount() {

    fprintf(stderr, "CountingStrategy: LevelCounts: %d, %d, %d\n", levelCount[0], levelCount[1], levelCount[2]);

    last_packets_per_second = 0;


    for (int i = 0; i < 3; i++)
    {
      last_packets_per_second += levelCount[i];
    }

    fprintf(stderr, "last_packets_per_second=%d\n", last_packets_per_second);


    double congestionPercentage = (double)(last_packets_per_second-110) / ((double)last_packets_per_second);



    if (last_packets_per_second != 0)
    {

      for (int i = 0; i < 3; i++)
      {

        levelProb[i] = levelProb[i] * (1-UPDATE_ALPHA) +
            (double)levelCount[i]/(double)last_packets_per_second * UPDATE_ALPHA;
      }

      dropProb[0] = dropProb[1] = dropProb[2] = 0.0;

      if (congestionPercentage > 0.0)
      {
        // calculate congestion drop percentage
        if (levelProb[2] > 0.0)
          dropProb[2] = congestionPercentage / levelProb[2];

        if (dropProb[2] > 1.0)
        {
          dropProb[2] = 1.0;

          if (levelProb[1] > 0.0)
            dropProb[1] = (congestionPercentage - levelProb[2]*dropProb[2]) / levelProb[1];
        }
      }

      fprintf(stderr, "CountingStrategy: Cong=%f, DropProb[2]=%f, DropProb[1]=%f\n", congestionPercentage, dropProb[2], dropProb[1]);




    }

    fprintf(stderr, "CountingStrategy: Probabilities: %f, %f, %f\n", levelProb[0], levelProb[1], levelProb[2]);


    levelCount.clear(); levelCount.reserve(3);
    levelCount[0] = levelCount[1] = levelCount[2] = 0;

    ResetStatisticsTimer = Simulator::Schedule(Seconds(1.0), &SVCCountingStrategy::resetLevelCount, this);
  }

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
void SVCCountingStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{
  Ptr<Packet> packet = Wire::FromInterest(interest);
  SVCLevelTag levelTag;
  DeadlineTag deadlineTag;

  uint64_t deadline = 0;

  bool deadlineTagExists = packet->PeekPacketTag(deadlineTag);
  if (deadlineTagExists)
  {
    //fprintf(stderr, "deadline tag found!\n");
    deadline = deadlineTag.Get();
  }

  if (deadline == 0)
  {
    deadline = 1000;
  }



  //check if enchancment layer
  bool tagExists = packet->PeekPacketTag(levelTag);

  int level = 0;
  if (tagExists)
  {
    level = levelTag.Get();
  }
  if (level == 16)
    level = 1;
  if (level == 32)
    level = 2;
  else
    level = 0;

  levelCount[level] = levelCount[level] + 1;

  // check if RandomNumber(0,1) < DropProbability
  // if yes --> drop
  // draw a random number
  UniformVariable rand (0,1);

  double val = rand.GetValue();

  if (val < dropProb[level])
  {
    // DROP
    //NS_LOG_UNCOND("Strategy: Dropping Interest " << interest->GetName ().toUri());

    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

    levelTag.Set (-1); // means packet dropped on purpose
    nack->GetPayload ()->AddPacketTag (levelTag);


    inface->SendInterest (nack);
    SVCCountingStrategy<Parent>::m_outNacks (nack, inface);
    return;
  }



  //fprintf(stderr, "CountingStrategy: LevelCounts: %d, %d, %d\n", levelCount[0], levelCount[1], levelCount[2]);

  super::OnInterest(inface,interest);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif // SVCCOUNTINGSTRATEGY_H
