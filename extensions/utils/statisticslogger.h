#ifndef STATISTICSLOGGER_H
#define STATISTICSLOGGER_H

#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <string.h>
#include <sstream>
#include "sdnparameters.h"
#include <map>

class StatisticsLogger
{
public:
    static StatisticsLogger* GetInstance();

    void AddNodeStatistics(int nodeId);
    void AddPeriodToNodeStatistics(int nodeId, int periodId, double avgSatisfactionRate, double avgRtt, std::string requestedContent);
    void SetAggregatedValuesOfNode(int nodeId);
    long GetSimulationRunId();
    void CompleteSimulationRun();
    static void Complete();
    std::string SendRequest(std::string url, std::string requestBody, std::string requestMethod);

private:
    StatisticsLogger(){}

    static StatisticsLogger *instance;
    static std::stringstream recv_data;

    void initialize();
    std::string CreateSimulationRun();
    std::string SendReqeust();
    static size_t CurlCallback(void *contents, size_t size, size_t nmemb, void *stream);

    bool initialized;
    long simulationRunId;
    CURL *ch;

    std::map<int, long> nodeIdMap;

};

#endif // STATISTICSLOGGER_H
