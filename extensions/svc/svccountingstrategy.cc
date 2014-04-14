#include "svccountingstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< SVCCountingStrategy<BestRoute> >;

typedef PerOutFaceLimits< SVCCountingStrategy<BestRoute> > PerOutFaceLimitsSVCCountingStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCCountingStrategyBestRoute);

typedef SVCCountingStrategy<BestRoute> SVCCountingStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(SVCCountingStrategyBestRoute);

typedef PerOutFaceLimits<SVCCountingStrategy<SmartFlooding> > PerOutFaceLimitsSVCCountingStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCCountingStrategySmartFlooding);

typedef SVCCountingStrategy<SmartFlooding> SVCCountingStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCCountingStrategySmartFlooding);

typedef PerOutFaceLimits<SVCCountingStrategy<Flooding> >PerOutFaceLimitsSVCCountingStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCCountingStrategyFlooding);

typedef SVCCountingStrategy<Flooding> SVCCountingStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCCountingStrategyFlooding);

} // namespace fw
} // namespace ndn
} // namespace ns3
