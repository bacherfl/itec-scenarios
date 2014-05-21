#include "svclivecountingstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< SVCLiveCountingStrategy<BestRoute> >;

typedef PerOutFaceLimits< SVCLiveCountingStrategy<BestRoute> > PerOutFaceLimitsSVCLiveCountingStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCLiveCountingStrategyBestRoute);

typedef SVCLiveCountingStrategy<BestRoute> SVCLiveCountingStrategyBestRoute;
NS_OBJECT_ENSURE_REGISTERED(SVCLiveCountingStrategyBestRoute);

typedef PerOutFaceLimits<SVCLiveCountingStrategy<SmartFlooding> > PerOutFaceLimitsSVCLiveCountingStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCLiveCountingStrategySmartFlooding);

typedef SVCLiveCountingStrategy<SmartFlooding> SVCLiveCountingStrategySmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCLiveCountingStrategySmartFlooding);

typedef PerOutFaceLimits<SVCLiveCountingStrategy<Flooding> >PerOutFaceLimitsSVCLiveCountingStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSVCLiveCountingStrategyFlooding);

typedef SVCLiveCountingStrategy<Flooding> SVCLiveCountingStrategyFlooding;
NS_OBJECT_ENSURE_REGISTERED(SVCLiveCountingStrategyFlooding);

} // namespace fw
} // namespace ndn
} // namespace ns3
