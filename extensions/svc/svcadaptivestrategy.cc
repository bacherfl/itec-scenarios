#include "svcadaptivestrategy.h"

#include "ns3-dev/ns3/ndn-fib.h"
#include "ns3-dev/ns3/ndn-fib-entry.h"
#include "ns3-dev/ns3/ndn-pit-entry.h"
#include "ns3-dev/ns3/ndn-interest.h"

#include <boost/foreach.hpp>

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< SVCAdaptiveStrategy<BestRoute> >;

typedef PerOutFaceLimits< SVCAdaptiveStrategy<BestRoute> > PerOutFaceLimitsSVCAdaptiveStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCAdaptiveStrategyBestRoute);

typedef SVCAdaptiveStrategy<BestRoute> SVCAdaptiveStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(SVCAdaptiveStrategyBestRoute);

typedef PerOutFaceLimits<SVCAdaptiveStrategy<SmartFlooding> > PerOutFaceLimitsSVCAdaptiveStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCAdaptiveStrategySmartFlooding);

typedef SVCAdaptiveStrategy<SmartFlooding> SVCAdaptiveStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCAdaptiveStrategySmartFlooding);

typedef PerOutFaceLimits<SVCAdaptiveStrategy<Flooding> >PerOutFaceLimitsSVCAdaptiveStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCAdaptiveStrategyFlooding);

typedef SVCAdaptiveStrategy<Flooding> SVCAdaptiveStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCAdaptiveStrategyFlooding);

} // namespace fw
} // namespace ndn
} // namespace ns3
