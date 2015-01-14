#ifndef SDNCONTROLLER_H
#define SDNCONTROLLER_H

#include "ns3-dev/ns3/log.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"
#include "ns3-dev/ns3/node.h"

#include "../../../ns-3/src/ndnSIM/model/ndn-global-router.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <curl/curl.h>

namespace ns3 {
namespace ndn {
namespace fw {
typedef struct route_t {
    std::vector<std::map<Ptr<Node>, Ptr<Face> > > endpoints;
} Route;

class SDNControlledStrategy;

class SDNController {
public:
    SDNController();

    static void CalculateRoutesForPrefix(Ptr<Node> start, const std::string &prefix);
    static void AddOrigins(std::string &prefix, Ptr<Node> producer);

    static void AddLink(Ptr<Node> a,
                        Ptr<Node> b,
                        std::map<std::string, std::string> channelAttributes,
                        std::map<std::string, std::string> deviceAttributes);

    static void AddLink(Ptr<Node> a,
                        Ptr<Node> b,
                        uint32_t faceId);

    static void RequestForUnknownPrefix(std::string &prefix);
    static void NodeReceivedNackOnFace(Ptr<Node>, Ptr<Face>);
    static void PerformNeo4jTrx(std::string url, std::string requestContent);

    static void registerForwarder(const SDNControlledStrategy *fwd, uint32_t nodeId);
    static void clearGraphDb();


private:
    static void PushRoute(Route route);


    typedef std::map<std::string, std::string > ChannelAttributes;
    typedef std::map<std::string, std::string > DeviceAttributes;
    typedef boost::tuples::tuple<Ptr<Node>, Ptr<Node>, ChannelAttributes, DeviceAttributes> IncidencyListEntry;
    Ptr<GlobalRouter> globalRouter;

    static std::map<uint32_t, const SDNControlledStrategy*> forwarders;

    static std::map<std::string, std::vector<Ptr<Node> > > contentOrigins;
    static std::vector<IncidencyListEntry> incidencyList;
    static const int IDX_NODE1 = 0;
    static const int IDX_NODE2 = 1;
    static const int IDX_CHANNEL_ATTRIBUTES = 2;
    static const int IDX_DEVICE_ATTRIBUTES = 3;

    static CURL *ch;


};
}
}
}

#endif // SDNCONTROLLER_H
