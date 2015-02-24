#include "sdncontroller.h"
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include "sdncontrolledstrategy.h"
#include "../sdnapp.h"

namespace ns3 {
namespace ndn {
namespace fw {
using namespace boost::tuples;
using namespace std;

std::map<std::string, std::vector<Ptr<Node> > > SDNController::contentOrigins;
std::map<uint32_t, SDNControlledStrategy*> SDNController::forwarders;
std::map<uint32_t, ns3::SDNApp*> SDNController::apps;
std::stringstream SDNController::recv_data;
CURL* SDNController::ch;
bool SDNController::isLargeNetwork = false;
std::vector<int> SDNController::leafNodes;
std::map<int, std::vector<Period *> > SDNController::periods;
std::map<int, int> SDNController::currentPeriodsPerAS;
std::map<int, int> SDNController::asSDNCaches;

SDNController::SDNController()
{
    globalRouter = Create<GlobalRouter>();
}

void SDNController::AppFaceAddedToNode(Ptr<Node> node)
{    
    int nodeId = node->GetId();

    std::stringstream statement;
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[l:LINK]->() SET l.faceId=l.faceId+1;";

    std::string data = PerformNeo4jTrx(statement.str(), curlCallback);

}

void SDNController::CalculateRoutesForPrefix(int startNodeId, const std::string &prefix)
{
    std::cout << "calculating route from node " << startNodeId << " for prefix " << prefix << "\n";

    //CalculateRoutesToAllSources(startNodeId, prefix);
    CalculateRouteToNearestSource(startNodeId, prefix);
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

void SDNController::CalculateRouteToNearestSource(int startNodeId, const std::string &prefix)
{
    std::stringstream statement;

    statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node)," <<
    "p = allShortestPaths((requester)-[:LINK*]->(server)) WHERE '" << prefix << "' in server.prefixes AND NOT server.nodeId = '" << startNodeId << "' return p ORDER BY length(p) LIMIT 1";

    std::string data = PerformNeo4jTrx(statement.str(), curlCallback);

    vector<Path *> paths = ParsePaths(data);

    if (paths.size() == 0) {
        statement.str("");
        statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (server:Node)," <<
        "p = allShortestPaths((requester)-[*]->(server)) WHERE '" << prefix << "' in server.prefixes AND NOT server.nodeId = '" << startNodeId << "' return p ORDER BY length(p) LIMIT 1";

        std::string data = PerformNeo4jTrx(statement.str(), curlCallback);
        vector<Path *> paths = ParsePaths(data);
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
    }
    else {
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
    }

}

void SDNController::CalculateRoutesToAllSources(int startNodeId, const std::string &prefix)
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

        std::string data = PerformNeo4jTrx(statement.str(), curlCallback);

        vector<Path *> paths = ParsePaths(data);

        if (paths.size() == 0) {
            CalculateAlternativeRoutesForPrefix(startNodeId, prefix, origins);
        }
        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
    }
}

std::string SDNController::GenerateExcludeStringForPathQuery(std::vector<std::string> origins, string target) {
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

void SDNController::CalculateAlternativeRoutesForPrefix(int startNodeId, const std::string &prefix, std::vector<std::string> origins)
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


        std::cout << statement.str() << "\n";

        std::string data = PerformNeo4jTrx(statement.str(), curlCallback);

        vector<Path *> paths = ParsePaths(data);

        for (int i = 0; i < paths.size(); i++)
        {
            PushPath(paths.at(i), prefix);
        }
    }
}

vector<string> SDNController::GetPrefixOrigins(const string &prefix)
{
    vector<string> origins;
    stringstream statement;
    statement << "MATCH (n:Node) WHERE '" << prefix << "' IN n.prefixes RETURN n;";

    string data = PerformNeo4jTrx(statement.str(), curlCallback);

    cout << data << "\n";

    Json::Reader reader;
    Json::Value root;

    std::cout << data << "\n";
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

vector<Path *> SDNController::ParsePaths(string data)
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

Path* SDNController::ParsePath(std::string data)
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

void SDNController::AddOrigins(std::string prefix, int prodId)
{
    std::stringstream statement;
    /*
    statement << "MATCH (n:Node) where n.nodeId = '" << prodId
                 << "' CREATE UNIQUE (n)-[:PROVIDES]->(p:Prefix {name:'" << prefix << "'}) RETURN p";
    */
    statement << "MATCH (n:Node) WHERE n.nodeId='" << prodId << "' "
                 << "SET n.prefixes = CASE WHEN NOT (HAS (n.prefixes)) "
                     << "THEN ['" << prefix << "'] "
                     << "ELSE n.prefixes + ['" << prefix << "'] "
                 << "END;";
    PerformNeo4jTrx(statement.str(), NULL);
}

void SDNController::PushPath(Path *p, const std::string &prefix)
{
    int cost = p->pathEntries.size();
    for (int i = 0; i < p->pathEntries.size(); i++)
    {
        PathEntry *pe = p->pathEntries.at(i);
        SDNControlledStrategy *strategy = forwarders[pe->start];
        strategy->PushRule(prefix, pe->face, cost);
        if (i < p->pathEntries.size() - 1) {
            if (pe->bandwidth > 0) {
                strategy->AssignBandwidth(
                        prefix,
                        pe->face,
                        pe->bandwidth / (strategy->getFlowsOfFace(pe->face).size() + 1)
                );
            }
            else {
                strategy->AssignBandwidth(
                        prefix,
                        pe->face,
                        1000000);
            }
        }
        cost--;
    }
    //LogChosenPath(p, prefix);
}

void SDNController::RecordFlow(int nodeId, int faceId, const std::string & name)
{
    std::stringstream statement;

    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'}) CREATE (n)-[:FLOW]->(f:FaceFlow {faceId: " << faceId << ", name:'" << name << "'});";

    PerformNeo4jTrx(statement.str(), curlCallback);
}

void SDNController::LinkFailure(int nodeId, int faceId, std::string name, double failureRate)
{
    std::cout << "Link failure \n";
    std::vector<string> statements;
    std::stringstream statement;

    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[f:LINK {startFace:"<< faceId <<"}]->(m) CREATE (n)-[f2:RED_LINK]->(m) SET f2 = f;";
    statements.push_back(statement.str());

    statement.str("");
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[f:LINK]->() WHERE f.startFace = " << faceId << " DELETE f;";
    statements.push_back(statement.str());

    PerformNeo4jTrx(statements, curlCallback);

    /*
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[f:LINK {startFace:"<< faceId <<"}]->() SET f.status='RED' , f.failureRate=" << failureRate << ";";
    */
    PerformNeo4jTrx(statement.str(), curlCallback);
    if (!isLargeNetwork)
    {
        CalculateRoutesForPrefix(nodeId, name);
        //FindAlternativePathBasedOnSatRate(nodeId, name);    //currently takes way too long for larger networks
    }
    else {
        CalculateRoutesForPrefix(nodeId, name);
    }
}

void SDNController::LinkRecovered(int nodeId, int faceId, std::string prefix, double failureRate)
{
    std::cout << "Link recovered \n";
    std::vector<string> statements;
    std::stringstream statement;

    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[f:RED_LINK {startFace:"<< faceId <<"}]->(m) CREATE (n)-[f2:LINK]->(m) SET f2 = f;";
    statements.push_back(statement.str());

    statement.str("");
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[f:RED_LINK {startFace:"<< faceId <<"}]->(m) DELETE f;";
    statements.push_back(statement.str());

    PerformNeo4jTrx(statements, curlCallback);

    //TODO: shift traffic to recovered link
}

void SDNController::FindAlternativePathBasedOnSatRate(int startNodeId, const std::string &prefix)
{
    std::stringstream statement;

    /*
    statement << "MATCH (n:Node{nodeId:'" << startNodeId << "'}), (m:Node), "
                 << "p = (n)-[*..6]->(m) "
                 << "WHERE '" << prefix << "' IN m.prefixes AND all (l in relationships(p) where l.failureRate < 0.3) "
                 << "AND ALL(n in nodes(p) WHERE "
                           << "1=length(filter(m in nodes(p) WHERE m=n))) "
                 << "RETURN p, length(p) as length, "
                 << "REDUCE(totalSatRate=1, l in relationships(p) | totalSatRate*(1-l.failureRate)) AS satRate "
                 << "ORDER BY satRate/length(p) DESC "
                 << "LIMIT 1;";
    */
    statement << "MATCH (n:Node{nodeId:'" << startNodeId << "'}), (m:Node), "
                 << "p = (n)-[*..6]->(m) "
                 << "WHERE '" << prefix << "' IN m.prefixes AND all (l in relationships(p) where l.failureRate < 0.3) "
                 << "AND ALL(n in nodes(p) WHERE "
                           << "1=length(filter(m in nodes(p) WHERE m=n))) "
                 << "RETURN p, length(p) as length, "
                 << "REDUCE(totalSatRate=1, l in relationships(p) | totalSatRate*(1-l.failureRate)) AS satRate "
                 << "LIMIT 1;";

    std::cout << statement.str() << "\n";
    std::string data = PerformNeo4jTrx(statement.str(), curlCallback);
    std::cout << data << "\n";

    Path *p = ParsePath(data);
    if (p != NULL)
    {
        PushPath(p, prefix);
    }
}

void SDNController::InstallBandwidthQueue(int nodeId, int faceId, std::string prefix)
{

}

void SDNController::LogChosenPath(Path p, const std::string &prefix)
{
    std::vector<std::string> statements;

    std::stringstream statement;
    for (int i = 0; i < p.pathEntries.size(); i++)
    {

        PathEntry *pe = p.pathEntries.at(i);
        statement << "MATCH (n:Node {nodeId:'" << pe->start << "'}), (m:Node {nodeId:'" << pe->end <<"'}) "
                     << "CREATE UNIQUE (n)-[:ROUTE {faceId:" << pe->face << ", prefix:'" << prefix << "'}]->(m);";

        statements.push_back(statement.str());
        statement.str("");
    }
    PerformNeo4jTrx(statements, NULL);
}

size_t SDNController::curlCallback(void *contents, size_t size, size_t nmemb, void *stream)
{
    for (int c = 0; c<size*nmemb; c++)
    {
        recv_data << ((char*)contents)[c];
    }

    return size*nmemb;
}

std::string SDNController::PerformNeo4jTrx(std::vector<std::string> statementsStr, size_t (*callback)(void*, size_t, size_t, void*))
{
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    for (int i = 0; i < statementsStr.size(); i++)
    {
        Json::Value statementObject = Json::Value(Json::objectValue);
        statementObject["statement"] = statementsStr.at(i);
        statements.append(statementObject);
    }

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    struct curl_slist *headers = NULL;
    if ((ch = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create curl handle");
        return "";
    }

    headers = curl_slist_append(headers, "Accept: application/json; charset=UTF-8");
    headers = curl_slist_append(headers, "Content-type: application/json");

    curl_easy_setopt(ch, CURLOPT_URL, "http://10.0.2.2:7474/db/data/transaction/commit");
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, writer.write(neo4jTrx).c_str());
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, callback);

    int rcode = curl_easy_perform(ch);

    /*
    if(rcode != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(rcode));
    */
    /* always cleanup */
    curl_easy_cleanup(ch);

    std::string ret(recv_data.str());
    recv_data.str("");

    return ret;
}

std::string SDNController::PerformNeo4jTrx(std::string statement, size_t (*callback)(void*, size_t, size_t, void*))
{
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);

    statementObject["statement"] = statement;

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    struct curl_slist *headers = NULL;
    if ((ch = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create curl handle");
        return "";
    }

    headers = curl_slist_append(headers, "Accept: application/json; charset=UTF-8");
    headers = curl_slist_append(headers, "Content-type: application/json");

    curl_easy_setopt(ch, CURLOPT_URL, "http://10.0.2.2:7474/db/data/transaction/commit");
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, writer.write(neo4jTrx).c_str());
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, callback);

    int rcode = curl_easy_perform(ch);

    /*
    if(rcode != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(rcode));
    */
    /* always cleanup */
    curl_easy_cleanup(ch);

    std::string ret(recv_data.str());
    recv_data.str("");

    return ret;
}

void SDNController::AddLink(Ptr<Node> a,
                            Ptr<Node> b,
                            std::map<std::string, std::string > channelAttributes,
                            std::map<std::string, std::string > deviceAttributes)
{
    int idA = a->GetId();
    int idB = b->GetId();

    std::stringstream statement;
    std::stringstream attributes;
    for (std::map<std::string, std::string>::iterator it = deviceAttributes.begin(); it != deviceAttributes.end(); it++)
    {
        if (it->first.compare("DataRate") == 0)
        {
            int mult = 1000;
            if (it->second.find("Mbps") != std::string::npos)
                mult = 1000000;
            std::string dataRate = it->second.substr(0, it->second.length() - 4);
            int bitrate = atoi(dataRate.c_str()) * mult;
            attributes << "," << it->first << ":" << bitrate;
        }
        else {
            attributes << "," << it->first << ":'" << it->second << "'";
        }

    }
    //cout << "Adding Link (" << idA << ")--(" << idB << "); \n";
    statement << "MATCH (a:Node {nodeId:'" << idA << "'}), (b:Node {nodeId:'" << idB << "'}), (a)-[l]-(b) DELETE l;";

    PerformNeo4jTrx(statement.str(), curlCallback);

    statement.str("");

    statement << "MERGE (a:Node {nodeId:'" << idA << "'}) " <<
                 "MERGE (b:Node {nodeId:'" << idB << "'}) " <<
                 "CREATE (a)-[:LINK {startFace:"
                    << getNumberOfFacesForNode(idA) << ", endFace:" << getNumberOfFacesForNode(idB) << attributes.str() << ", failureRate: 0.0"
                    << "} ]->(b) " <<
                 "CREATE (a)<-[:LINK {startFace:"
                    << getNumberOfFacesForNode(idB) << ", endFace:" << getNumberOfFacesForNode(idA) << attributes.str() << ", failureRate: 0.0"
                    << "} ]-(b) RETURN a";


    /*
    statement << "MERGE (a:Node {nodeId:'" << idA << "'}) " <<
                 "MERGE (b:Node {nodeId:'" << idB << "'}) " <<
                 "CREATE (a)-[:LINK]->(fa:Face {faceId:" << getNumberOfFacesForNode(idA) << attributes.str() << "})-[:LINK]->(fb:Face {faceId:" << getNumberOfFacesForNode(idB) << attributes.str() << "})-[:LINK]->(b) RETURN a";

    */

    PerformNeo4jTrx(statement.str(), curlCallback);
}

void SDNController::clearGraphDb()
{
    std::stringstream statement;
    //statement << "MATCH (n:Node)-[r]-(), (p:Prefix)-[r2]-(), (p2:RequestedPrefix)-[r3]-() DELETE n, r, p, r2, p2, r3;";
    statement << "MATCH (n)-[r]-() DELETE n, r";

    PerformNeo4jTrx(statement.str(), NULL);
}

void SDNController::SetLinkBitrate(int nodeId, int faceId, uint64_t bitrate)
{
    std::stringstream statement;
    statement << "MATCH (:Node {nodeId:'" << nodeId << "'})-[]-(f:Face{faceId:" << faceId << "}) SET f.bitrate=" << bitrate <<";";

    PerformNeo4jTrx(statement.str(), NULL);
}

int SDNController::getNumberOfFacesForNode(uint32_t nodeId)
{
    std::stringstream statement;
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[l]-() RETURN n, count(*)";

    std::string data = PerformNeo4jTrx(statement.str(), curlCallback);

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

void SDNController::AddLink(Ptr<Node> a,
                    Ptr<Node> b,
                    uint32_t faceId)
{
    int idA = a->GetId();
    int idB = b->GetId();

    std::stringstream statement;
    statement << "MERGE (a:Node {nodeId:'" << idA << "'}) " <<
                 "MERGE (b:Node {nodeId:'" << idB << "'}) " <<
                 "CREATE (a)-[l:LINK {faceId:" << faceId <<"}]->(b) RETURN a,l";


    PerformNeo4jTrx(statement.str(), NULL);
}

void SDNController::DidReceiveValidNack(int nodeId, int faceId, std::string name)
{

}

void SDNController::registerForwarder(SDNControlledStrategy *fwd, uint32_t nodeId)
{
    forwarders[nodeId] = fwd;
}

void SDNController::RegisterApp(ns3::SDNApp *app, uint32_t nodeId)
{
    apps[nodeId] = app;
    //app->RequestContent("/itec/bunny_2s_480p_only/bunny_2s_100kbit/bunny_2s1.m4s", 1000000);
    //AddOrigins("/itec/bunny_2s_480p_only/bunny_2s_100kbit/bunny_2s1.m4s", nodeId);
}

void SDNController::LogRequest(uint32_t nodeId, std::string prefix)
{
    stringstream statement;
    statement << "MERGE (requester:Node{nodeId:'" << nodeId << "'}) " <<
                 "MERGE (p:RequestedPrefix {prefix:'" << prefix <<"', time: " << (int) (Simulator::Now().GetSeconds()) << "}) " <<
                 "CREATE (requester)-[:REQUEST]->(p);";

    PerformNeo4jTrx(statement.str(), curlCallback);
}

void SDNController::RequestForUnknownPrefix(std::string &prefix)
{

}

void SDNController::NodeReceivedNackOnFace(Ptr<Node>, Ptr<Face>)
{

}

void SDNController::SetASNumberOfClient(int clientId, int asNumber)
{
    stringstream statement;
    statement << "MATCH (n:Node {nodeId:'" << clientId << "'}) SET n.as = " << asNumber <<";";
    PerformNeo4jTrx(statement.str(), curlCallback);
}

void SDNController::SetASNumberOfSDNCache(int clientId, int asNumber)
{
    asSDNCaches[asNumber] = clientId;
}

void SDNController::SetPeriodPopularityConfig(std::string configFilePath)
{
    periods = PeriodFactory::GetInstance()->GetPeriods(configFilePath);

    PlanNextPeriods();
}

void SDNController::PlanNextPeriods()
{
    typedef map<int, vector<Period *> > Periods;
    for (Periods::iterator it = periods.begin(); it != periods.end(); it++) {
        currentPeriodsPerAS[it->first] = 0;
        PlanPeriodForAS(it->first);
    }
}

void SDNController::PlanPeriodForAS(int asId)
{
    Period *p = periods[asId].at(++currentPeriodsPerAS[asId] % periods[asId].size());

    //get most popular content for the next period
    double max = 0.0;
    string contentName;
    for (map<string, double>::iterator it = p->popularities.begin(); it != p->popularities.end(); it++) {
        if (it->second >= max) {
            contentName = it->first;
            max = it->second;
        }
    }

    SDNApp *sdnCache = apps[asSDNCaches[asId]];
    sdnCache->RequestContent(contentName, 750000, p->contentSizes[contentName]);

    Simulator::Schedule(Seconds(p->length), &SDNController::PlanPeriodForAS, asId);
}

}
}
}
