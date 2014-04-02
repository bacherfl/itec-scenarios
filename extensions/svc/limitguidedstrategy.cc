#include "limitguidedstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< LimitsGuidedStrategy<BestRoute> >;

typedef PerOutFaceLimits< LimitsGuidedStrategy<BestRoute> > PerOutFaceLimitsLimitsGuidedStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsLimitsGuidedStrategyBestRoute);

typedef LimitsGuidedStrategy<BestRoute> LimitsGuidedStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(LimitsGuidedStrategyBestRoute);

typedef PerOutFaceLimits<LimitsGuidedStrategy<SmartFlooding> > PerOutFaceLimitsLimitsGuidedStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsLimitsGuidedStrategySmartFlooding);

typedef LimitsGuidedStrategy<SmartFlooding> LimitsGuidedStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(LimitsGuidedStrategySmartFlooding);

typedef PerOutFaceLimits<LimitsGuidedStrategy<Flooding> >PerOutFaceLimitsLimitsGuidedStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsLimitsGuidedStrategyFlooding);

typedef LimitsGuidedStrategy<Flooding> LimitsGuidedStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(LimitsGuidedStrategyFlooding);


typedef LimitsGuidedStrategy<PerOutFaceLimits <BestRoute> > LimitsGuidedStrategyPerOutFaceLimitsBestRoute;
NS_OBJECT_ENSURE_REGISTERED(LimitsGuidedStrategyPerOutFaceLimitsBestRoute);

} // namespace fw
} // namespace ndn
} // namespace ns3

