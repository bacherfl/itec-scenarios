#include "sdncontentrequester.h"
#include "ns3/ndn-name.h"

using namespace ns3;
using namespace ns3::ndn;

SDNContentRequester::SDNContentRequester(IDownLoader *app, std::string name, int dataRate):
    app(app),
    name(name)
{
    double interestsPerSecond = (dataRate + 0.0) / (8 * (4096 + 50));
    interestInterval = 1000.0 / interestsPerSecond;
    chunkNr = 0;
}

void SDNContentRequester::SendNextInterest()
{
    std::stringstream nameStr;
    nameStr << name;
    app->SendInterest(nameStr.str(), chunkNr);
    chunkNr++;

    nextInterestEvent = Simulator::Schedule(MilliSeconds(interestInterval), &SDNContentRequester::SendNextInterest, this);
}

void SDNContentRequester::RequestContent()
{    
    SendNextInterest();
}

void SDNContentRequester::Stop()
{
    Simulator::Cancel(nextInterestEvent);
}
