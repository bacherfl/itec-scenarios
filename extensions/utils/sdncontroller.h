#ifndef SDNCONTROLLER_H
#define SDNCONTROLLER_H

#include "ns3-dev/ns3/log.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"

#include "../../../ns-3/src/ndnSIM/model/ndn-global-router.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace ns3 {
namespace ndn {

typedef struct route_t {
    std::vector<std::map<Ptr<Node>, Ptr<Face> > > endpoints;
} Route;

class SDNController {
public:
    SDNController();

    static void CalculateRoutesForPrefix(Ptr<Node> start, const std::string &prefix);
    static void AddOrigins(std::string &prefix, Ptr<Node> producer);

    static void AddLink(Ptr<Node> a,
                        Ptr<Node> b,
                        std::map<std::string, AttributeValue& > channelAttributes,
                        std::map<std::string, AttributeValue& > deviceAttributes);

    static void RequestForUnknownPrefix(std::string &prefix);
    static void NodeReceivedNackOnFace(Ptr<Node>, Ptr<Face>);


private:
    static void PushRoute(Route route);


    typedef std::map<std::string, AttributeValue& > ChannelAttributes;
    typedef std::map<std::string, AttributeValue& > DeviceAttributes;
    typedef boost::tuples::tuple<Ptr<Node>, Ptr<Node>, ChannelAttributes, DeviceAttributes> IncidencyListEntry;
    Ptr<GlobalRouter> globalRouter;

    static std::map<std::string, std::vector<Ptr<Node> > > contentOrigins;
    static std::vector<IncidencyListEntry> incidencyList;
    static const int IDX_NODE1 = 0;
    static const int IDX_NODE2 = 1;
    static const int IDX_CHANNEL_ATTRIBUTES = 2;
    static const int IDX_DEVICE_ATTRIBUTES = 3;



};
}
}


#endif // SDNCONTROLLER_H
