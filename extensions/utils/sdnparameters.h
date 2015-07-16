#ifndef SDNPARAMETERS_H
#define SDNPARAMETERS_H

#include <string>

class SDNParameters
{
public:
    SDNParameters();

    static const double MIN_SAT_RATIO;
    static const double SHORTEST_PATH_FRACTION;
    static const int INTEREST_INTERVAL;
    static const int SDN_CACHE_DOWNLOAD_RATE;
    static const bool ENABLE_PREFETCHING;
    static const bool LOG_SIMULATION;
    static const double BW_ASSIGNMENT_ADJ;
    static const std::string SIMULATION_SESSION;
    static const std::string TOPOLOGY;
    static const std::string STRATEGY_NAME;
    static const std::string STRATEGY_SDN_CACHE;
    static const std::string STRATEGY_SDN;
    static const std::string STRATEGY_BESTROUTE;
    static const std::string STRATEGY_FLOODING;
    static const std::string SESSION_DESCRIPTION;
    static const int ZIPF_DISTRIBUTION_ALPHA;
};

#endif // SDNPARAMETERS_H
