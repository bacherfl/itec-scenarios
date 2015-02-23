#include "periodclient.h"
#include "jsoncpp/json/json.h"

#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/ndn-app-face.h"
#include "ns3/random-variable.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "utils/sdncontroller.h"
#include <fstream>
#include "periodfactory.h"


namespace ns3 {

using namespace ndn;

NS_OBJECT_ENSURE_REGISTERED (PeriodClient);

TypeId PeriodClient::GetTypeId()
{
    static TypeId tid = TypeId("PeriodClient")
            .SetParent<ndn::StatisticsConsumer>()
            .AddConstructor<PeriodClient>()
            .AddAttribute ("Region", "Region",
                           IntegerValue (0),
                           MakeIntegerAccessor(&PeriodClient::m_region),
                           MakeIntegerChecker<int32_t>())
            .AddAttribute ("ConfigFile","Config File",
                           StringValue ("simulation-periods.json"),
                           MakeStringAccessor (&PeriodClient::m_configFile),
                           MakeStringChecker ());

    return tid;
}

void PeriodClient::StartApplication()
{
    ndn::StatisticsConsumer::StartApplication();
    init();
}

void PeriodClient::init()
{
    this->periods = PeriodFactory::GetInstance()->GetPeriodsForRegion(m_configFile, m_region);

    int i = 0;
    for (Periods::iterator it = this->periods.begin(); it != this->periods.end(); it++) {
        Period *p = (*it);
        std::cout << "Period " << i << ":" << std::endl;
        for (std::map<std::string, double>::iterator it2 = p->popularities.begin(); it2 != p->popularities.end(); it2++) {
            std::cout << "  Popularity(" << it2->first << ") = " << it2->second << std::endl;
        }
        i++;
    }

    currentPeriod = 0;
    Simulator::ScheduleNow(&PeriodClient::StartNextPeriod, this);
    //Simulator::Schedule(Seconds(5.0), &PeriodClient::LogCurrentInterest, this);
}

void PeriodClient::LogCurrentInterest()
{
    ns3::ndn::fw::SDNController::LogRequest(GetNode()->GetId(), this->currentContentName);
    Simulator::Schedule(Seconds(5.0), &PeriodClient::LogCurrentInterest, this);
}

void PeriodClient::StartNextPeriod()
{
    Period *p = periods.at(currentPeriod++ % periods.size());

    //select a content based on the popularity values for the current period
    DetermineContentNameForPeriod(p);
    std::cout << "selected content: " << currentContentName << std::endl;

    if (requester != NULL) {
        requester->Stop();
    }
    currentSeqNr = 0;

    int contentSize = p->contentSizes[currentContentName];
    //set some default size if no size info available in the configuration file
    if (contentSize == 0)
        contentSize = 5000000;

    requester = new SDNContentRequester(
                        this,
                        currentContentName,
                        1000000,
                        contentSize
                ); //TODO make download rate configurable
    requester->RequestContent();

    nextEvent = Simulator::Schedule(Seconds(p->length), &PeriodClient::StartNextPeriod, this);
}

void PeriodClient::DetermineContentNameForPeriod(Period *p)
{
    currentContentName = "";
    int rnd = rand() % 101;

    double sum = 0;
    for (std::map<std::string, double>::iterator it = p->popularities.begin(); it != p->popularities.end(); it++) {
        if (sum + it->second >= rnd) {
            currentContentName = it->first;
            return;
        }
        sum += it->second;
    }
}

void PeriodClient::StopApplication()
{
    Simulator::Cancel(nextEvent);
    if (requester != NULL)
        requester->Stop();
    ndn::StatisticsConsumer::StopApplication();
}

void PeriodClient::OnInterest(Ptr<const ndn::Interest> interest)
{
    ndn::StatisticsConsumer::OnInterest(interest);
}

void PeriodClient::OnData(Ptr<const ndn::Data> contentObject)
{
    ndn::StatisticsConsumer::OnData(contentObject);
}

void PeriodClient::SendInterest(std::string name, uint32_t seqNum)
{    
    if (name.compare("") == 0)
        return;

    Ptr<ndn::Name> prefix = Create<ndn::Name> (name);
    prefix->appendSeqNum(seqNum);

    UniformVariable rand(0, std::numeric_limits<uint32_t>::max());

    Ptr<ndn::Interest> interest = Create<ndn::Interest>();
    interest->SetName(prefix);
    interest->SetInterestLifetime(Seconds(1.0));
    interest->SetNonce(rand.GetValue());

    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest(interest);
    this->nrSentInterests++;
}

void PeriodClient::OnDownloadFinished(std::string prefix)
{

}

void PeriodClient::WillSendOutInterest(uint32_t sequenceNumber)
{
    ndn::StatisticsConsumer::WillSendOutInterest(sequenceNumber);
}

}
