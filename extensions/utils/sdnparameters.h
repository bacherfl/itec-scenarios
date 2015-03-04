#ifndef SDNPARAMETERS_H
#define SDNPARAMETERS_H

class SDNParameters
{
public:
    SDNParameters();

    static const double MIN_SAT_RATIO;
    static const double SHORTEST_PATH_FRACTION;
    static const int INTEREST_INTERVAL;
    static const int SDN_CACHE_DOWNLOAD_RATE;
};

#endif // SDNPARAMETERS_H
