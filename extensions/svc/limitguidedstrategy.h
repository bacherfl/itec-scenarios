#ifndef LIMITGUIDEDSTRATEGY_H
#define LIMITGUIDEDSTRATEGY_H

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

#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"

#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>

#include <stdio.h>

#include "svcleveltag.h"
#include "svcbitratetag.h"
#include "facepitstatistic.h"

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::LimitsGuidedStrategy
template<class Parent>
class LimitsGuidedStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  LimitsGuidedStrategy (){}

protected:

  virtual bool CanSendOutInterest ( Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);

  static LogComponent g_log;
};

template<class Parent>
LogComponent LimitsGuidedStrategy<Parent>::g_log = LogComponent (LimitsGuidedStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId LimitsGuidedStrategy<Parent>::GetTypeId ()
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::LimitsGuidedStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <LimitsGuidedStrategy> ();
  return tid;
}

template<class Parent>
std::string LimitsGuidedStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".LimitsGuidedStrategy";
}

template<class Parent>
bool LimitsGuidedStrategy<Parent>::CanSendOutInterest ( Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  if(!super::CanSendOutInterest(inFace, outFace, interest, pitEntry))
  {
    fprintf(stderr, "Limit exceeded adaptation needed\n");
    return false;
  }

  return true;
}

} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // LimitsGuidedStrategy_H
