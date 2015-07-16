#include "sdnparameters.h"

const std::string SDNParameters::STRATEGY_SDN_CACHE = "sdn-cache";
const std::string SDNParameters::STRATEGY_SDN = "sdn";
const std::string SDNParameters::STRATEGY_BESTROUTE = "bestRoute";
const std::string SDNParameters::STRATEGY_FLOODING = "flooding";

const double SDNParameters::MIN_SAT_RATIO = 0.95;
const int SDNParameters::INTEREST_INTERVAL = 50;
const double SDNParameters::SHORTEST_PATH_FRACTION = 0.85;
const double SDNParameters::BW_ASSIGNMENT_ADJ = 0.9;
const int SDNParameters::SDN_CACHE_DOWNLOAD_RATE = 1300000;
const bool SDNParameters::ENABLE_PREFETCHING = true;
const bool SDNParameters::LOG_SIMULATION = true;
const std::string SDNParameters::TOPOLOGY = "brite_configs/brite_low_bw.conf";
const std::string SDNParameters::SIMULATION_SESSION = "8";
const std::string SDNParameters::STRATEGY_NAME = SDNParameters::STRATEGY_SDN_CACHE;
const std::string SDNParameters::SESSION_DESCRIPTION = "Prefetching while NW load is low";
const int SDNParameters::ZIPF_DISTRIBUTION_ALPHA = 2;


SDNParameters::SDNParameters()
{
}
