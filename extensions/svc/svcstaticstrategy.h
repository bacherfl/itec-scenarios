#ifndef SVCSTATICSTRATEGY_H
#define SVCSTATICSTRATEGY_H


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


#include "svcleveltag.h"
#include "svcbitratetag.h"

#include "../utils/deadlinetag.h"


#include <stdio.h>


#define DEFAULT_MAX_LEVEL 0


namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SVCCountingStrategy
template<class Parent>
class SVCStaticStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SVCStaticStrategy () : super()
  {
    fprintf(stderr, "SVCStaticStrategy activated...\n");
  }

  virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);
protected:
  int m_maxLevel;
  static LogComponent g_log;
};

template<class Parent>
LogComponent SVCStaticStrategy<Parent>::g_log = LogComponent (SVCStaticStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId SVCStaticStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SVCStaticStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SVCStaticStrategy> ()
      .template AddAttribute("MaxLevelAllowed", "The maximum level allowed as a positive integer >= 0",
                    IntegerValue(DEFAULT_MAX_LEVEL),
                    MakeIntegerAccessor(&SVCStaticStrategy<Parent>::m_maxLevel),
                             MakeIntegerChecker<int32_t>());
  return tid;
}

template<class Parent>
std::string SVCStaticStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".SVCStaticStrategy";
}




template<class Parent>
void SVCStaticStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{
  SVCLevelTag levelTag;
  // get the actual packet so we can access tags
  Ptr<Packet> packet = Wire::FromInterest (interest);
  bool svcLevelTagExists = packet->PeekPacketTag (levelTag);

  int level = 0;

  //TODO fix levels
  if (svcLevelTagExists)
  {
    level = levelTag.Get ();
  }


  // check if level is allowed
  if (level > m_maxLevel)
  {
      fprintf(stderr, "Dropping packet with level %d\n", level);
    // DROP
    NS_LOG_INFO("Strategy: Dropping Interest " << interest->GetName ().toUri());
    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (ndn::Interest::NACK_GIVEUP_PIT); // set this since ndn changes it anyway to this.

    levelTag.Set (-1); // means packet dropped on purpose
    nack->GetPayload ()->AddPacketTag (levelTag);

    inface->SendInterest (nack);
    SVCStaticStrategy<Parent>::m_outNacks (nack, inface);
    // nack sent - we dont need anything else --> return
    return;
  }

  // let parent continue with the interest
  super::OnInterest(inface,interest);
}


}
}
}



#endif // SVCSTATICSTRATEGY_H
