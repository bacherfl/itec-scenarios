#ifndef SDNINTERESTHANDLER_H
#define SDNINTERESTHANDLER_H

#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-data.h"
#include "ns3-dev/ns3/ndn-app.h"
#include "jsoncpp/json/json.h"

namespace ns3 {
namespace ndn {
namespace sdn {
class SDNInterestHandler : public SimpleRefCount<SDNInterestHandler>
{
public:
    SDNInterestHandler(Ptr<App> owner);

    Ptr<Data> SendControllerDiscoveryResponse(Ptr<const Interest> interest);
    Ptr<Data> ProcessInterest(Ptr<const ns3::ndn::Interest> interest);

protected:
    int sequenceNumber;
    Ptr<App> owner;

};

} //namespace sdn
} //namespace ndn
} //namespace ns3
#endif // SDNINTERESTHANDLER_H
