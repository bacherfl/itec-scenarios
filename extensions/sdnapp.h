#ifndef SDNAPP_H
#define SDNAPP_H

#include "ns3/ndnSIM/apps/ndn-producer.h"
#include "idownloader.h"
#include <vector>

class SDNContentRequester;

namespace ns3 {

class SDNApp : public ndn::Producer, public IDownLoader
{
public:
    static TypeId GetTypeId();

    virtual void StartApplication();
    virtual void StopApplication();

    virtual void OnInterest(Ptr<const ndn::Interest> interest);
    virtual void OnData(Ptr<const ndn::Data> contentObject);

    void RequestContent(const std::string &name, int dataRate);
    void RequestContent(const std::string &name, int dataRate, int contentSize);

    virtual void SendInterest(std::string name, uint32_t seqNum);
    virtual void OnDownloadFinished(std::string prefix);

protected:
    void RegisterAtController();

private:
    std::vector<SDNContentRequester *> contentRequesters;
};

}

#endif // SDNAPP_H
