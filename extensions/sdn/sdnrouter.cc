#include "sdnrouter.h"

namespace ns3 {
namespace ndn {
namespace fw {

//template class PerOutFaceLimits< SDNRouter<BestRoute> >;
/*
typedef PerOutFaceLimits< SDNRouter<BestRoute> > PerOutFaceLimitsSDNRouterBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSDNRouterBestRoute);

typedef SDNRouter<BestRoute> SDNRouterBestRoute;
NS_OBJECT_ENSURE_REGISTERED(SDNRouterBestRoute);

typedef PerOutFaceLimits<SDNRouter<SmartFlooding> > PerOutFaceLimitsSDNRouterSmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSDNRouterSmartFlooding);

typedef SDNRouter<SmartFlooding> SDNRouterSmartFlooding;
NS_OBJECT_ENSURE_REGISTERED(SDNRouterSmartFlooding);

typedef PerOutFaceLimits<SDNRouter<Flooding> >PerOutFaceLimitsSDNRouterFlooding;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSDNRouterFlooding);

typedef SDNRouter<Flooding> SDNRouterFlooding;
NS_OBJECT_ENSURE_REGISTERED(SDNRouterFlooding);
*/

/*typedef PerOutFaceLimits< SDNRouter<Nacks> > PerOutFaceLimitsSDNRouterNacks;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsSDNRouterNacks);*/

typedef SDNRouter<Nacks> SDNRouterNacks;
NS_OBJECT_ENSURE_REGISTERED(SDNRouterNacks);

} // namespace fw
} // namespace ndn
} // namespace ns3

