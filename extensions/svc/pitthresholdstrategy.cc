#include "pitthresholdstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< PitThresholdStrategy<BestRoute> >;

typedef PerOutFaceLimits< PitThresholdStrategy<BestRoute> > PerOutFaceLimitsPitThresholdStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsPitThresholdStrategyBestRoute);

typedef PitThresholdStrategy<BestRoute> PitThresholdStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PitThresholdStrategyBestRoute);

typedef PerOutFaceLimits<PitThresholdStrategy<SmartFlooding> > PerOutFaceLimitsPitThresholdStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsPitThresholdStrategySmartFlooding);

typedef PitThresholdStrategy<SmartFlooding> PitThresholdStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PitThresholdStrategySmartFlooding);

typedef PerOutFaceLimits<PitThresholdStrategy<Flooding> >PerOutFaceLimitsPitThresholdStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsPitThresholdStrategyFlooding);

typedef PitThresholdStrategy<Flooding> PitThresholdStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PitThresholdStrategyFlooding);

} // namespace fw
} // namespace ndn
} // namespace ns3
