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
#include <fstream>


namespace ns3 {

using namespace ndn;

NS_OBJECT_ENSURE_REGISTERED (PeriodClient);

TypeId PeriodClient::GetTypeId()
{
    static TypeId tid = TypeId("PeriodClient")
            .SetParent<ndn::App>()
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
    ndn::App::StartApplication();
    init();
}

void PeriodClient::init()
{
    std::ifstream file(m_configFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string configStr = buffer.str();

    Json::Reader reader;
    Json::Value root;

    bool parsingSuccessful = reader.parse(configStr, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << configStr <<  "\n";
        return;
    }

    periodLength = root["periodLength"].asInt();

    Json::Value periods = root["Periods"];

    for (int i = 0; i < periods.size(); i++) {
        Json::Value period = periods[i];
        Period *p = new Period;
        p->length = periodLength;
        Json::Value regionStatistics = period["regionStatistics"];
        for (int j = 0; j < regionStatistics.size(); j++) {
            if (regionStatistics[j]["region"].asInt() == m_region) {
                Json::Value stats = regionStatistics[j]["statistics"];

                for (int x = 0; x < stats.size(); x++) {
                    std::string name = stats[x]["name"].asString();
                    double popularity = stats[x]["popularity"].asDouble();
                    p->popularities[name] = popularity;
                }
            }            
        }
        this->periods.push_back(p);
    }
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

    requester = new SDNContentRequester(this, currentContentName, 1000000); //TODO make download rate configurable
    requester->RequestContent();

    nextEvent = Simulator::Schedule(Seconds(periodLength), &PeriodClient::StartNextPeriod, this);
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
    ndn::App::StopApplication();
}

void PeriodClient::OnInterest(Ptr<const ndn::Interest> interest)
{
    ndn::App::OnInterest(interest);
}

void PeriodClient::OnData(Ptr<const ndn::Data> contentObject)
{
    ndn::App::OnData(contentObject);
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
}

}
