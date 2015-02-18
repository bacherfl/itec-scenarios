#ifndef PERIODCLIENT_H
#define PERIODCLIENT_H

#include "ns3/ndn-app.h"
#include "idownloader.h"
#include "utils/sdncontentrequester.h"
#include <map>

namespace ns3 {

struct Period {
    int length;
    std::map<std::string, double> popularities;
};

class PeriodClient : public ndn::App, public IDownLoader
{
public:
    static TypeId GetTypeId();

    virtual void StartApplication();
    virtual void StopApplication();

    virtual void OnInterest(Ptr<const ndn::Interest> interest);
    virtual void OnData(Ptr<const ndn::Data> contentObject);

    virtual void SendInterest(std::string name, uint32_t seqNum);

private:
    std::string m_configFile;
    uint32_t m_region;
    typedef std::vector<Period *> Periods;
    Periods periods;
    int currentPeriod;
    int periodLength;
    ns3::EventId nextEvent;

    std::string currentContentName;
    int currentSeqNr;
    SDNContentRequester *requester;

    void init();
    void StartNextPeriod();
    void DetermineContentNameForPeriod(Period *p);
    //void Request
};

}

#endif // PERIODCLIENT_H
