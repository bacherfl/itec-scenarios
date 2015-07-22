#include "sdnroutingmodule.h"
#include <jsoncpp/json/json.h>

using namespace std;

namespace ns3 {
namespace ndn {
namespace fw {

SDNRoutingModule::SDNRoutingModule()
{
    neo4jInterface = new Neo4jInterface;
}

std::vector<Path *> SDNRoutingModule::CalculateRoutesForPrefix(int startNodeId, const std::string &prefix)
{
    //std::cout << "calculating route from node " << startNodeId << " for prefix " << prefix << "\n";

    //CalculateRoutesToAllSources(startNodeId, prefix);
    return CalculateRouteToNearestSource(startNodeId, prefix);
    /*
    if (getNumberOfFacesForNode(startNodeId) == 1) {
        //node is a leaf node

        statement.str("");
        statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}) CREATE (requester)-[:REQUEST]->(p:RequestedPrefix {prefix:'" << prefix <<"', time: " << Simulator::Now().GetSeconds() << "});";

        PerformNeo4jTrx(statement.str(), curlCallback);
    }
    */
    //AddOrigins(prefix, startNodeId);
}

std::vector<Path *> SDNRoutingModule::CalculateRouteToNearestSource(int startNodeId, const std::string &prefix)
{
    std::stringstream statement;

    statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node)," <<
    "p = allShortestPaths((requester)-[:LINK*]->(server)) WHERE '" << prefix << "' in server.prefixes AND NOT server.nodeId = '" << startNodeId << "' return p ORDER BY length(p) ASC";

    std::string data = neo4jInterface->PerformNeo4jTrx(statement.str());

    vector<Path *> paths = ParsePaths(data);

    if (paths.size() == 0) {
        statement.str("");
        statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node)," <<
        "p = allShortestPaths((requester)-[*]->(server)) WHERE '" << prefix << "' in server.prefixes AND NOT server.nodeId = '" << startNodeId << "' return p ORDER BY length(p) ASC LIMIT 1";

        std::string data = neo4jInterface->PerformNeo4jTrx(statement.str());
        vector<Path *> paths = ParsePaths(data);
        return paths;
        /*
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
        */
    }
    else {
        return paths;
        /*
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
        */
    }

}

vector<Path *> SDNRoutingModule::CalculateRoutesToAllSources(int startNodeId, const std::string &prefix)
{
    std::stringstream statement;

    vector<string> origins = GetPrefixOrigins(prefix);

    vector<Path *> paths;
    int minLength = INT_MAX;
    for (int i = 0; i < origins.size(); i++)
    {
        if (atoi(origins.at(i).c_str()) == startNodeId)
            continue;
        string exclStr = GenerateExcludeStringForPathQuery(origins, origins.at(i));

        statement.str("");
        if (origins.size() > 1) {
            statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node{nodeId:'" << origins.at(i) << "'})," <<
                         "p = allShortestPaths((requester)-[:LINK*]->(server)) WHERE ALL (n IN nodes(p) WHERE NOT n.nodeId IN " << exclStr << ") return p;";
        }
        else {
            statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node{nodeId:'" << origins.at(i) << "'})," <<
                         "p = allShortestPaths((requester)-[:LINK*]->(server)) return p;";
        }


        std::cout << statement.str() << "\n";

        std::string data = neo4jInterface->PerformNeo4jTrx(statement.str());

        vector<Path *> paths = ParsePaths(data);

        if (paths.size() == 0) {
            return CalculateAlternativeRoutesForPrefix(startNodeId, prefix, origins);
        }
        return paths;
        /*
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
        */
    }
}

std::string SDNRoutingModule::GenerateExcludeStringForPathQuery(std::vector<std::string> origins, string target)
{
    stringstream exclStr;
    if (origins.size() > 1) {
        vector<string> excl;
        for (int j = 0; j < origins.size(); j++) {
            if (origins.at(j).compare(target) != 0) {
                excl.push_back(origins.at(j));
            }
        }

        exclStr << "[";
        for (int i = 0; i < excl.size(); i++) {
            exclStr << "'" << excl.at(i) << "'";
            if (i < excl.size() -1)
                exclStr << ",";
        }
        exclStr << "]";
    }
    return exclStr.str();
}

std::vector<Path *> SDNRoutingModule::CalculateAlternativeRoutesForPrefix(int startNodeId, const std::string &prefix, std::vector<std::string> origins)
{
    std::stringstream statement;
    for (int i = 0; i < origins.size(); i++)
    {
        if (atoi(origins.at(i).c_str()) == startNodeId)
            continue;
        string exclStr = GenerateExcludeStringForPathQuery(origins, origins.at(i));

        statement.str("");
        if (origins.size() > 1) {
            statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node{nodeId:'" << origins.at(i) << "'})," <<
                         "p = allShortestPaths((requester)-[:LINK*]->(server)) WHERE ALL (n IN nodes(p) WHERE NOT n.nodeId IN " << exclStr << ") return p;";
        }
        else {
            statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node{nodeId:'" << origins.at(i) << "'})," <<
                         "p = allShortestPaths((requester)-[:LINK*]->(server)) return p;";
        }


        //std::cout << statement.str() << "\n";

        std::string data = neo4jInterface->PerformNeo4jTrx(statement.str());

        vector<Path *> paths = ParsePaths(data);

        return paths;
        /*
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
        */
    }
}

vector<Path *> SDNRoutingModule::ParsePaths(string data)
{
    vector<Path *> ret;
    Json::Reader reader;
    Json::Value root;

    //std::cout << data << "\n";
    bool parsingSuccessful = reader.parse(data, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << data <<  "\n";
        return ret;
    }

    Json::Value paths = root["results"][0]["data"][0]["row"];

    for (int i = 0; i < paths.size(); i++)
    {
        Json::Value path = paths[i];
        //std::cout << "path length: " << path.size() << "\n";

        bool firstNode = true;
        PathEntry *pe;
        Path *p = new Path;
        if (!path.isNull())
        {
            for (int i = 0; i < path.size() - 1; i += 2)
            {
                pe = new PathEntry;
                Json::Value startNode = path[i];
                Json::Value face = path[i+1];
                Json::Value endNode = path[i+2];
                pe->start = atoi(startNode["nodeId"].asCString());
                pe->face = face["startFace"].asInt();
                pe->end = atoi(endNode["nodeId"].asCString());
                pe->bandwidth = face["DataRate"].asInt();
                p->pathEntries.push_back(pe);
            }
            pe = new PathEntry;
            pe->start = p->pathEntries.at(p->pathEntries.size() - 1)->end;
            pe->face = getNumberOfFacesForNode(pe->start);
            pe->end = -1;   //App Face
            p->pathEntries.push_back(pe);
            ret.push_back(p);
        }
    }
    return ret;
}

Path* SDNRoutingModule::ParsePath(std::string data)
{
    Json::Reader reader;
    Json::Value root;

    std::cout << data << "\n";
    bool parsingSuccessful = reader.parse(data, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << data <<  "\n";
        return NULL;
    }

    Json::Value path = root["results"][0]["data"][0]["row"][0];

    std::cout << "path length: " << path.size() << "\n";

    bool firstNode = true;
    PathEntry *pe;
    Path *p = new Path;
    if (!path.isNull())
    {

        for (int i = 0; i < path.size() - 1; i += 2)
        {
            pe = new PathEntry;
            Json::Value startNode = path[i];
            Json::Value face = path[i+1];
            Json::Value endNode = path[i+2];
            pe->start = atoi(startNode["nodeId"].asCString());
            pe->face = face["startFace"].asInt();
            pe->end = atoi(endNode["nodeId"].asCString());
            pe->bandwidth = face["DataRate"].asInt();
            p->pathEntries.push_back(pe);
        }
        pe = new PathEntry;
        pe->start = p->pathEntries.at(p->pathEntries.size() - 1)->end;
        pe->face = getNumberOfFacesForNode(pe->start);
        pe->end = -1;   //App Face
        p->pathEntries.push_back(pe);
    }
    return p;
}

vector<string> SDNRoutingModule::GetPrefixOrigins(const string &prefix)
{
    vector<string> origins;
    stringstream statement;
    statement << "MATCH (n:Node) WHERE '" << prefix << "' IN n.prefixes RETURN n;";

    string data = neo4jInterface->PerformNeo4jTrx(statement.str());

    //cout << data << "\n";

    Json::Reader reader;
    Json::Value root;

    //std::cout << data << "\n";
    bool parsingSuccessful = reader.parse(data, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << data <<  "\n";
        return origins;
    }

    Json::Value originsJson = root["results"][0]["data"];

    for (int i = 0; i < originsJson.size(); i++)
    {
        string origin = originsJson[i]["row"][0]["nodeId"].asString();
        origins.push_back(origin);
    }

    return origins;
}

int SDNRoutingModule::getNumberOfFacesForNode(uint32_t nodeId)
{
    std::stringstream statement;
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[l]-() RETURN n, count(*)";

    std::string data = neo4jInterface->PerformNeo4jTrx(statement.str());

    //result of curl call is in recv_data
    Json::Reader reader;
    Json::Value root;
    bool parsingSuccessful = reader.parse(data, root);
    if (!parsingSuccessful) {
        return 0;
    }
    Json::Value nrFaces = root["results"][0]["data"][0]["row"][1];
    return nrFaces.asInt() / 2;
    //return nrFaces.asInt();
}

}
}
}
