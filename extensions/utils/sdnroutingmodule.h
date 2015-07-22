#ifndef SDNROUTINGMODULE_H
#define SDNROUTINGMODULE_H

#include "ns3-dev/ns3/log.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"
#include "ns3-dev/ns3/node.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <curl/curl.h>

#include "sdnparameters.h"
#include "neo4jinterface.h"

namespace ns3 {
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

class SDNRoutingModule
{
public:
    SDNRoutingModule();

    std::vector<Path *> CalculateRoutesForPrefix(int startNodeId, const std::string &prefix);
    std::vector<Path *> CalculateRoutesToAllSources(int startNodeId, const std::string &prefix);
    std::vector<Path *> CalculateRouteToNearestSource(int startNodeId, const std::string & prefix);
    std::vector<Path *> CalculateAlternativeRoutesForPrefix(int startNodeId, const std::string &prefix, std::vector<std::string> origins);
    std::vector<Path *> FindAlternativePathBasedOnSatRate(int startNodeId, const std::string &prefix);
    std::vector<Path *> ParsePaths(std::string data);
    std::vector<std::string> GetPrefixOrigins(const std::string &prefix);
    std::string GenerateExcludeStringForPathQuery(std::vector<std::string> origins, std::string target);
    int getNumberOfFacesForNode(uint32_t nodeId);
    Path* ParsePath(std::string data);

private:
    Neo4jInterface *neo4jInterface;
};

}
}
}

#endif // SDNROUTINGMODULE_H
