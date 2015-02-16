#ifndef SDNCONTENTREQUESTER_H
#define SDNCONTENTREQUESTER_H

#include "../sdnapp.h"

class SDNContentRequester
{
public:
    SDNContentRequester(ns3::SDNApp *app, std::string name, int dataRate);

    void RequestContent(const std::string &name, int dataRate);

private:
    void SendNextInterest();

    ns3::SDNApp *app;
    std::string name;
    double interestInterval;
    int chunkNr;
};

#endif // SDNCONTENTREQUESTER_H
