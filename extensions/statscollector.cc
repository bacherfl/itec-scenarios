#include "statscollector.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

// NS_LOG_COMPONENT_DEFINE ("ndn.fw.StatsCollector");

namespace ns3 {
namespace ndn {
namespace fw {

template<class Parent>
LogComponent StatsCollector<Parent>::g_log = LogComponent (StatsCollector<Parent>::GetLogName ().c_str ());


template<class Parent>
TypeId StatsCollector<Parent>::GetTypeId ()
{
  static TypeId tid = TypeId ((Parent::GetTypeId ().GetName ()+"::StatsCollector").c_str ())
    .SetGroupName ("Ndn")
    .template SetParent<Parent> ()
    .template AddConstructor< StatsCollector<Parent> > ();
  return tid;
}

template<class Parent>
std::string StatsCollector<Parent>::GetLogName ()
{
  return super::GetLogName () + ".StatsCollector";
}

template<class Parent>
void StatsCollector<Parent>::DidSendOutInterest (Ptr<Face> inFace, Ptr<Face> outFace,
                                        Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  super::DidSendOutInterest (inFace, outFace, interest, pitEntry);
  //fprintf(stderr, "StatsCollector: send out interest: %s\n", interest->GetName().toUri().c_str());
  //todo
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#include <ns3/ndnSIM/model/fw/best-route.h>
#include <ns3/ndnSIM/model/fw/flooding.h>
#include <ns3/ndnSIM/model/fw/smart-flooding.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::StatsCollector::DynamicLimits::SimpleLimits
template class StatsCollector< BestRoute >;
typedef StatsCollector< BestRoute > StatsCollectorBestRoute;
NS_OBJECT_ENSURE_REGISTERED(StatsCollectorBestRoute);

template class StatsCollector< Flooding >;
typedef StatsCollector< Flooding > StatsCollectorFlooding;
NS_OBJECT_ENSURE_REGISTERED(StatsCollectorFlooding);

template class StatsCollector< SmartFlooding >;
typedef StatsCollector< SmartFlooding > StatsCollectorSmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(StatsCollectorSmartFlooding);


} // namespace fw
} // namespace ndn
} // namespace ns3
