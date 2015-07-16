#include "statisticslogger.h"

using namespace std;

StatisticsLogger* StatisticsLogger::instance = NULL;
std::stringstream StatisticsLogger::recv_data;

StatisticsLogger* StatisticsLogger::GetInstance()
{
    if (instance == NULL) {
        instance = new StatisticsLogger;
        instance->initialize();
    }
    return instance;
}

void StatisticsLogger::initialize()
{
    initialized = true;
    CreateSimulationRun();
}

std::string StatisticsLogger::CreateSimulationRun()
{
    if (!SDNParameters::LOG_SIMULATION)
        return "";
    //TODO get parameters from ParameterConfig class and create SimulationRun JSON object to send to REST service
    Json::Value request = Json::Value(Json::objectValue);
    Json::Value simulationRun = Json::Value(Json::objectValue);
    simulationRun["sdnCacheDownloadRate"] = SDNParameters::SDN_CACHE_DOWNLOAD_RATE;
    simulationRun["minSatRatio"] = SDNParameters::MIN_SAT_RATIO;
    simulationRun["shortestPathSelectionProbability"] = SDNParameters::SHORTEST_PATH_FRACTION;
    simulationRun["interestInterval"] = SDNParameters::INTEREST_INTERVAL;
    simulationRun["enablePrefetching"] = SDNParameters::ENABLE_PREFETCHING;
    simulationRun["valid"] = SDNParameters::LOG_SIMULATION;
    simulationRun["strategyName"] = SDNParameters::STRATEGY_NAME;
    simulationRun["topology"] = SDNParameters::TOPOLOGY;
    simulationRun["bwAssignmentAdj"] = SDNParameters::BW_ASSIGNMENT_ADJ;
    simulationRun["sessionName"] = SDNParameters::SIMULATION_SESSION;
    simulationRun["description"] = SDNParameters::SESSION_DESCRIPTION;
    simulationRun["zipfDistributionAlpha"] = SDNParameters::ZIPF_DISTRIBUTION_ALPHA;
    request["simulationRun"] = simulationRun;

    Json::StyledWriter writer;
    string strSimId = SendRequest(string("http://10.0.2.2:8080/statistics"), writer.write(simulationRun), string("POST"));
    simulationRunId = atol(strSimId.c_str());
}

void StatisticsLogger::AddNodeStatistics(int nodeId)
{
    if (!SDNParameters::LOG_SIMULATION)
        return;
    stringstream nodeName;
    nodeName << nodeId;

    Json::Value nodeStatistics = Json::Value(Json::objectValue);
    nodeStatistics["name"] = nodeName.str();

    Json::StyledWriter writer;

    stringstream url;
    url << "http://10.0.2.2:8080/statistics/" << simulationRunId << "/node";
    string strNodeId = SendRequest(url.str(), writer.write(nodeStatistics), string("POST"));
    nodeIdMap[nodeId] = atol(strNodeId.c_str());
}

void StatisticsLogger::AddPeriodToNodeStatistics(int nodeId, int periodId, double avgSatisfactionRate, double avgRtt, std::string requestedContent)
{
    if (!SDNParameters::LOG_SIMULATION)
        return;
    long id = nodeIdMap[nodeId];

    Json::Value period = Json::Value(Json::objectValue);
    period["seqNr"] = periodId;
    period["requestedContent"] = requestedContent;
    period["averageSatisfactionRate"] = avgSatisfactionRate;
    period["averageRtt"] = avgRtt;

    Json::StyledWriter writer;

    stringstream url;
    url << "http://10.0.2.2:8080/statistics/nodes/" << id << "/period";

    SendRequest(url.str(), writer.write(period), string("POST"));
}

void StatisticsLogger::SetAggregatedValuesOfNode(int nodeId)
{
    if (!SDNParameters::LOG_SIMULATION)
        return;
    long id = nodeIdMap[nodeId];

    stringstream url;
    url << "http://10.0.2.2:8080/statistics/nodes/" << id << "/aggr";

    SendRequest(url.str(), "", string("PUT"));
}

void StatisticsLogger::CompleteSimulationRun()
{
    if (!SDNParameters::LOG_SIMULATION)
        return;
    stringstream url;
    url << "http://10.0.2.2:8080/statistics/" << simulationRunId << "/complete";
    SendRequest(url.str(), "", string("PUT"));
}

void StatisticsLogger::Complete()
{
    GetInstance()->CompleteSimulationRun();
}

long StatisticsLogger::GetSimulationRunId()
{
    return simulationRunId;
}

std::string StatisticsLogger::SendRequest(std::string url, std::string requestBody, std::string requestMethod)
{
    struct curl_slist *headers = NULL;
    if ((ch = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create curl handle");
        return "";
    }

    headers = curl_slist_append(headers, "Accept: application/json; charset=UTF-8");
    headers = curl_slist_append(headers, "Content-type: application/json");

    curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, requestMethod.c_str());
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, CurlCallback);

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

    cout << ret << endl;

    return ret;
}

size_t StatisticsLogger::CurlCallback(void *contents, size_t size, size_t nmemb, void *stream)
{
    for (int c = 0; c<size*nmemb; c++)
    {
        recv_data << ((char*)contents)[c];
    }

    return size*nmemb;
}
