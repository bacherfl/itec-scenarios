#ifndef SDNAPP_H
#define SDNAPP_H

#include "ns3/ndn-app.h"
#include <vector>

class SDNContentRequester;

namespace ns3 {

class SDNApp : public ndn::App
{
public:
    static TypeId GetTypeId();

    virtual void StartApplication();
    virtual void StopApplication();

    virtual void OnInterest(Ptr<const ndn::Interest> interest);
    virtual void OnData(Ptr<const ndn::Data> contentObject);

    void RequestContent(const std::string &name, int dataRate);
    void SendInterest(std::string name, uint32_t seqNum);

protected:
    void RegisterAtController();

private:
    std::vector<SDNContentRequester *> contentRequesters;
};

}

#endif // SDNAPP_H
