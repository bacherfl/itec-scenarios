#ifndef SDNCONTENTREQUESTER_H
#define SDNCONTENTREQUESTER_H

#include "../idownloader.h"
#include "ns3/simulator.h"

class SDNContentRequester
{
public:
    SDNContentRequester(IDownLoader *app, std::string name, int dataRate);
    SDNContentRequester(IDownLoader *app, std::string name, int dataRate, double contentSize);

    void RequestContent();
    void Stop();

private:
    void InitDataRate(int dataRate);
    void SendNextInterest();

    IDownLoader *app;
    std::string name;
    double interestInterval;
    int maxChunk;
    int chunkNr;
    ns3::EventId nextInterestEvent;
};

#endif // SDNCONTENTREQUESTER_H
