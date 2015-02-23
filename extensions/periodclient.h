#ifndef PERIODCLIENT_H
#define PERIODCLIENT_H

#include "ns3/ndn-app.h"
#include "idownloader.h"
#include "utils/statisticsconsumer.h"
#include "utils/sdncontentrequester.h"
#include "period.h"
#include <map>

namespace ns3 {

class PeriodClient : public ndn::StatisticsConsumer, public IDownLoader
{
public:
    static TypeId GetTypeId();

    virtual void StartApplication();
    virtual void StopApplication();

    virtual void OnInterest(Ptr<const ndn::Interest> interest);
    virtual void OnData(Ptr<const ndn::Data> contentObject);

    virtual void SendInterest(std::string name, uint32_t seqNum);
    virtual void OnDownloadFinished(std::string prefix);
    virtual void WillSendOutInterest(uint32_t sequenceNumber);

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
    void LogCurrentInterest() ;
    //void Request
};

}

#endif // PERIODCLIENT_H
