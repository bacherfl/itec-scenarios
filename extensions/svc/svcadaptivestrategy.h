#ifndef SVCADAPTIVESTRATEGY_H
#define SVCADAPTIVESTRATEGY_H

#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"

#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"

#include "../stats.h"
#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::Stats::SVCAdaptiveStrategy
template<class Parent>
class SVCAdaptiveStrategy:
    public Stats< Parent >
{
public:

  typedef Stats< Parent > super;

  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SVCAdaptiveStrategy () {}

protected:
  static LogComponent g_log;
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

} // namespace fw
} // namespace ndn
} // namespace ns3
#endif // SVCADAPTIVESTRATEGY_H
