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

#include "../periodfactory.h"
#include "sdnparameters.h"

namespace ns3 {

class SDNApp;

namespace ndn {
namespace fw {

typedef struct route_t {
    std::vector<std::map<Ptr<Node>, Ptr<Face> > > endpoints;
} Route;

typedef struct path_entry_t {
    int start;
    int face;
    int end;
    int bandwidth;
} PathEntry;

typedef struct path_t {
    std::vector<PathEntry*> pathEntries;
} Path;

class SDNControlledStrategy;

class SDNController {
public:
    SDNController();

    static void CalculateRoutesForPrefix(int startNodeId, const std::string &prefix);
    static void CalculateRoutesToAllSources(int startNodeId, const std::string &prefix);
    static void CalculateRouteToNearestSource(int startNodeId, const std::string & prefix);
    static void CalculateAlternativeRoutesForPrefix(int startNodeId, const std::string &prefix, std::vector<std::string> origins);
    static void FindAlternativePathBasedOnSatRate(int startNodeId, const std::string &prefix);
    static std::vector<Path *> ParsePaths(std::string data);
    static Path* ParsePath(std::string data);
    static void AddOrigins(std::string prefix, int prodId);
    static std::vector<std::string> GetPrefixOrigins(const std::string &prefix);

    static void AddLink(Ptr<Node> a,
                        Ptr<Node> b,
                        std::map<std::string, std::string> channelAttributes,
                        std::map<std::string, std::string> deviceAttributes);

    static void AddLink(Ptr<Node> a,
                        Ptr<Node> b,
                        uint32_t faceId);

    static void RequestForUnknownPrefix(std::string &prefix);
    static void NodeReceivedNackOnFace(Ptr<Node>, Ptr<Face>);
    static std::string PerformNeo4jTrx(std::string requestContent, size_t (*callback)(void*, size_t, size_t, void*));
    static std::string PerformNeo4jTrx(std::vector<std::string> statements, size_t (*callback)(void*, size_t, size_t, void*));

    static void registerForwarder(SDNControlledStrategy *fwd, uint32_t nodeId);
    static void RegisterApp(ns3::SDNApp *app, uint32_t nodeId);
    static void clearGraphDb();
    static int getNumberOfFacesForNode(uint32_t nodeId);
    static size_t curlCallback(void *ptr, size_t size, size_t nmemb, void *stream);
    static void AppFaceAddedToNode(Ptr<Node> node);
    static void DidReceiveValidNack(int nodeId, int faceId, std::string name);
    static void LogChosenPath(Path p, const std::string &prefix);
    static void LinkFailure(int nodeId, int faceId, std::string name, double failureRate);
    static void InstallBandwidthQueue(int nodeId, int faceId, std::string prefix);
    static void SetLinkBitrate(int nodeId, int faceId, uint64_t bitrate);
    static void LinkRecovered(int nodeId, int faceId, std::string prefix, double failureRate);
    static void RecordFlow(int nodeId, int faceId, const std::string & name);
    static std::string GenerateExcludeStringForPathQuery(std::vector<std::string> origins, std::string target);
    static void LogRequest(uint32_t nodeId, std::string prefix);

    static void SetASNumberOfClient(int clientId, int asNumber);
    static void SetASNumberOfSDNCache(int clientId, int asNumber);

    static void SetPeriodPopularityConfig(std::string configFilePath);
    static void PlanNextPeriods();
    static void PlanPeriodForAS(int asId);

    static bool isLargeNetwork;

private:
    static void PushPath(Path *p, const std::string &prefix);


    typedef std::map<std::string, std::string > ChannelAttributes;
    typedef std::map<std::string, std::string > DeviceAttributes;
    typedef boost::tuples::tuple<Ptr<Node>, Ptr<Node>, ChannelAttributes, DeviceAttributes> IncidencyListEntry;
    Ptr<GlobalRouter> globalRouter;

    static std::map<uint32_t, SDNControlledStrategy*> forwarders;
    static std::map<uint32_t, ns3::SDNApp*> apps;

    static std::map<int, int> asSDNCaches;

    static std::map<std::string, std::vector<Ptr<Node> > > contentOrigins;
    static std::vector<IncidencyListEntry> incidencyList;
    static const int IDX_NODE1 = 0;
    static const int IDX_NODE2 = 1;
    static const int IDX_CHANNEL_ATTRIBUTES = 2;
    static const int IDX_DEVICE_ATTRIBUTES = 3;

    static std::stringstream recv_data;
    static std::vector<int> leafNodes;

    static CURL *ch;

    static std::map<int, std::vector<Period *> > periods;
    static std::map<int, int> currentPeriodsPerAS;

};
}
}
}

#endif // SDNCONTROLLER_H
