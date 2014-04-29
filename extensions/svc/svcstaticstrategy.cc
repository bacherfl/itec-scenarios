#include "svcstaticstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< SVCStaticStrategy<BestRoute> >;

typedef PerOutFaceLimits< SVCStaticStrategy<BestRoute> > PerOutFaceLimitsSVCStaticStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCStaticStrategyBestRoute);

typedef SVCStaticStrategy<BestRoute> SVCStaticStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(SVCStaticStrategyBestRoute);

typedef PerOutFaceLimits<SVCStaticStrategy<SmartFlooding> > PerOutFaceLimitsSVCStaticStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCStaticStrategySmartFlooding);

typedef SVCStaticStrategy<SmartFlooding> SVCStaticStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCStaticStrategySmartFlooding);

typedef PerOutFaceLimits<SVCStaticStrategy<Flooding> >PerOutFaceLimitsSVCStaticStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCStaticStrategyFlooding);

typedef SVCStaticStrategy<Flooding> SVCStaticStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCStaticStrategyFlooding);

} // namespace fw
} // namespace ndn
} // namespace ns3
