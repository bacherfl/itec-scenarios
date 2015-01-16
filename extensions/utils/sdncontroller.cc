#include "sdncontroller.h"
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include "sdncontrolledstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {
using namespace boost::tuples;
using namespace std;

std::map<std::string, std::vector<Ptr<Node> > > SDNController::contentOrigins;
std::map<uint32_t, SDNControlledStrategy*> SDNController::forwarders;
std::stringstream SDNController::recv_data;
CURL* SDNController::ch;

SDNController::SDNController()
{
    globalRouter = Create<GlobalRouter>();
}

void SDNController::AppFaceAddedToNode(Ptr<Node> node)
{
    int nodeId = node->GetId();

    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[l:LINK]->() SET l.faceId=l.faceId+1;";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    std::string data = PerformNeo4jTrx(writer.write(neo4jTrx), curlCallback);

}

void SDNController::CalculateRoutesForPrefix(int startNodeId, const std::string &prefix)
{
    std::cout << "calculating route from node " << startNodeId << " for prefix " << prefix << "\n";
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;
    statement << "MATCH (requester:Node{nodeId:'" << startNodeId << "'}), (content:Prefix{name:'" << prefix << "'})," <<
                 "p = shortestPath((requester)-[*]->(content)) return p;";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    std::string data = PerformNeo4jTrx(writer.write(neo4jTrx), curlCallback);

    Json::Reader reader;
    Json::Value root;

    std::cout << data << "\n";
    bool parsingSuccessful = reader.parse(data, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << data <<  "\n";
        return;
    }

    Json::Value path = root["results"][0]["data"][0]["row"][0];

    std::cout << "path length: " << path.size() << "\n";

    bool firstNode = true;
    PathEntry *pe;
    Path p;
    if (!path.isNull())
    {
        for (int i = 0; i < path.size(); i++) {
            if (firstNode)
            {
                pe = new PathEntry;
                Json::Value node = path[i];
                if (!node["nodeId"].isNull())
                {
                    pe->start = atoi(node["nodeId"].asCString());
                    firstNode = false;
                }
            }
            else {
                Json::Value node = path[i];
                if (!node["faceId"].isNull())
                {
                    pe->face = node["faceId"].asInt();
                }
                else if (!node["nodeId"].isNull())
                {
                    pe->end = atoi(node["nodeId"].asCString());
                    p.pathEntries.push_back(pe);
                    pe = new PathEntry;
                    pe->start = atoi(node["nodeId"].asCString());
                }
            }
        }
    }
    std::stringstream str;
    str << "\n" << p.pathEntries.size() << "\n";
    fprintf(stderr, "%s", str.str().c_str());
    for (int i = 0; i < p.pathEntries.size(); i++)
    {
        std::cout << "[" << p.pathEntries.at(i)->start << ", " << p.pathEntries.at(i)->face << ", " << p.pathEntries.at(i)->end << "] \n";
    }
    PushPath(p, prefix);
}

void SDNController::AddOrigins(std::string &prefix, Ptr<Node> producer)
{
    vector<Ptr<Node> > producersForPrefix = contentOrigins[prefix];
    producersForPrefix.push_back(producer);

    //insert content location into graph database

    int prodId = producer->GetId();

    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;
    statement << "MATCH (n:Node) where n.nodeId = '" << prodId
                 << "' CREATE UNIQUE (n)-[:PROVIDES]->(p:Prefix {name:'" << prefix << "'}) RETURN p";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    PerformNeo4jTrx(writer.write(neo4jTrx), NULL);
}

void SDNController::PushPath(Path p, const std::string &prefix)
{
    for (int i = 0; i < p.pathEntries.size(); i++)
    {
        PathEntry *pe = p.pathEntries.at(i);
        SDNControlledStrategy *strategy = forwarders[pe->start];
        strategy->PushRule(prefix, pe->face);
    }
}

size_t SDNController::curlCallback(void *contents, size_t size, size_t nmemb, void *stream)
{
    for (int c = 0; c<size*nmemb; c++)
    {
        recv_data << ((char*)contents)[c];
    }

    return size*nmemb;
}

std::string SDNController::PerformNeo4jTrx(std::string requestContent, size_t (*callback)(void*, size_t, size_t, void*))
{
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
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, requestContent.c_str());
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

    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;

    statement << "MERGE (a:Node {nodeId:'" << idA << "'}) " <<
                 "MERGE (b:Node {nodeId:'" << idB << "'}) " <<
                 "CREATE (a)-[:LINK {faceId:" << getNumberOfFacesForNode(idA) << "} ]->(b) " <<
                 "CREATE (a)<-[:LINK {faceId:" << getNumberOfFacesForNode(idB) << "} ]-(b) RETURN a";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    PerformNeo4jTrx(writer.write(neo4jTrx), NULL);
}

void SDNController::clearGraphDb()
{
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;
    statement << "MATCH (n:Node)-[r]-(), (p:Prefix)-[r2]-() DELETE n, r, p, r2;";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    PerformNeo4jTrx(writer.write(neo4jTrx), NULL);
}

int SDNController::getNumberOfFacesForNode(uint32_t nodeId)
{
    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;
    statement << "MATCH (n:Node {nodeId:'" << nodeId << "'})-[l:LINK]-() RETURN n, count(*)";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    std::string data = PerformNeo4jTrx(writer.write(neo4jTrx), curlCallback);

    std::cout << "result for GetNumberOfFacesForNode(): " << data << "\n";

    //result of curl call is in recv_data
    Json::Reader reader;
    Json::Value root;
    bool parsingSuccessful = reader.parse(data, root);
    if (!parsingSuccessful) {
        return 0;
    }
    Json::Value nrFaces = root["results"][0]["data"][0]["row"][1];
    return nrFaces.asInt() / 2;
}

void SDNController::AddLink(Ptr<Node> a,
                    Ptr<Node> b,
                    uint32_t faceId)
{
    int idA = a->GetId();
    int idB = b->GetId();

    Json::Value neo4jTrx = Json::Value(Json::objectValue);
    Json::Value  statements = Json::Value(Json::arrayValue);

    Json::Value statementObject = Json::Value(Json::objectValue);
    std::stringstream statement;
    statement << "MERGE (a:Node {nodeId:'" << idA << "'}) " <<
                 "MERGE (b:Node {nodeId:'" << idB << "'}) " <<
                 "CREATE (a)-[l:LINK {faceId:" << faceId <<"}]->(b) RETURN a,l";

    statementObject["statement"] = statement.str();

    statements.append(statementObject);

    neo4jTrx["statements"] = statements;

    Json::StyledWriter writer;

    PerformNeo4jTrx(writer.write(neo4jTrx), NULL);
}

void SDNController::registerForwarder(SDNControlledStrategy *fwd, uint32_t nodeId)
{
    forwarders[nodeId] = fwd;
}

void SDNController::RequestForUnknownPrefix(std::string &prefix)
{

}

void SDNController::NodeReceivedNackOnFace(Ptr<Node>, Ptr<Face>)
{

}

}
}
}
