#include "sdncontentrequester.h"
#include "ns3/ndn-name.h"

using namespace ns3;
using namespace ns3::ndn;

SDNContentRequester::SDNContentRequester(ns3::SDNApp *app, std::string name, int dataRate):
    app(app),
    name(name)
{
    double interestsPerSecond = (dataRate + 0.0) / (8 * (4096 + 50));
    interestInterval = 1000.0 / interestsPerSecond;
    chunkNr = 0;
}

void SDNContentRequester::SendNextInterest()
{
    std::cout << chunkNr << std::endl;
    std::stringstream nameStr;
    if (chunkNr < 10)
        nameStr << name << "/%0" << chunkNr;
    else
        nameStr << name << "/%" << chunkNr;
    app->SendInterest(nameStr.str());
    chunkNr++;

    Simulator::Schedule(MilliSeconds(interestInterval), &SDNContentRequester::SendNextInterest, this);
}

void SDNContentRequester::RequestContent(const std::string &name, int dataRate)
{
    SendNextInterest();
}
