#ifndef SDNCONTENTREQUESTER_H
#define SDNCONTENTREQUESTER_H

#include "../idownloader.h"
#include "ns3/simulator.h"

class SDNContentRequester
{
public:
    SDNContentRequester(IDownLoader *app, std::string name, int dataRate);

    void RequestContent();
    void Stop();

private:
    void SendNextInterest();

    IDownLoader *app;
    std::string name;
    double interestInterval;
    int chunkNr;
    ns3::EventId nextInterestEvent;
};

#endif // SDNCONTENTREQUESTER_H
