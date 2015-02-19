#include "sdncontentrequester.h"
#include "ns3/ndn-name.h"

using namespace ns3;
using namespace ns3::ndn;

SDNContentRequester::SDNContentRequester(IDownLoader *app, std::string name, int dataRate):
    app(app),
    name(name),
    chunkNr(0),
    maxChunk(-1)
{
    InitDataRate(dataRate);
}

SDNContentRequester::SDNContentRequester(IDownLoader *app, std::string name, int dataRate, double contentSize):
    app(app),
    name(name),
    chunkNr(0)
{
    InitDataRate(dataRate);
    maxChunk = ceil(contentSize / 4096.0);
}

void SDNContentRequester::InitDataRate(int dataRate)
{
    double interestsPerSecond = (dataRate + 0.0) / (8 * (4096 + 50));
    interestInterval = 1000.0 / interestsPerSecond;
}

void SDNContentRequester::SendNextInterest()
{
    std::stringstream nameStr;
    nameStr << name;
    app->SendInterest(nameStr.str(), chunkNr);
    chunkNr++;
    if ((chunkNr < maxChunk) || (maxChunk == -1)) {
        nextInterestEvent = Simulator::Schedule(MilliSeconds(interestInterval), &SDNContentRequester::SendNextInterest, this);
    }
}

void SDNContentRequester::RequestContent()
{    
    SendNextInterest();
}

void SDNContentRequester::Stop()
{
    Simulator::Cancel(nextInterestEvent);
}
