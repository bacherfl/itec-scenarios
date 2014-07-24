#include "rttbasedcachingstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< RTTBasedCachingStrategy<BestRoute> >;

typedef PerOutFaceLimits< RTTBasedCachingStrategy<BestRoute> > PerOutFaceLimitsRTTBasedCachingStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsRTTBasedCachingStrategyBestRoute);

typedef RTTBasedCachingStrategy<BestRoute> RTTBasedCachingStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(RTTBasedCachingStrategyBestRoute);

typedef PerOutFaceLimits<RTTBasedCachingStrategy<SmartFlooding> > PerOutFaceLimitsRTTBasedCachingStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsRTTBasedCachingStrategySmartFlooding);

typedef RTTBasedCachingStrategy<SmartFlooding> RTTBasedCachingStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(RTTBasedCachingStrategySmartFlooding);

typedef PerOutFaceLimits<RTTBasedCachingStrategy<Flooding> >PerOutFaceLimitsRTTBasedCachingStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsRTTBasedCachingStrategyFlooding);

typedef RTTBasedCachingStrategy<Flooding> RTTBasedCachingStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(RTTBasedCachingStrategyFlooding);

} // namespace fw
} // namespace ndn
} // namespace ns3
