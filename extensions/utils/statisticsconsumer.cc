#include "statisticsconsumer.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"


NS_LOG_COMPONENT_DEFINE ("ndn.StatisticsConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (StatisticsConsumer);

StatisticsConsumer::StatisticsConsumer ()
{
    nrSentInterests = 0;
    nrSatisfiedInterests = 0;
    nrTimeouts = 0;
    nrNacks = 0;
}

StatisticsConsumer::~StatisticsConsumer()
{

}

//register NS-3 Type
TypeId
StatisticsConsumer::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::ndn::StatisticsConsumer")
            .SetGroupName ("Ndn")
            .SetParent<ndn::ConsumerCbr> ()
            .AddConstructor<StatisticsConsumer> ();

    return tid;
}

void StatisticsConsumer::WillSendOutInterest(uint32_t sequenceNumber)
{
    nrSentInterests++;
    //NS_LOG_DEBUG ("Sending out interest " << sequenceNumber);
    ndn::ConsumerCbr::WillSendOutInterest(sequenceNumber);
}

void StatisticsConsumer::OnNack (const Ptr<const Interest> interest)
{
    nrNacks++;
    //NS_LOG_DEBUG ("Received NACK for Interest " << interest->GetName());
    ndn::ConsumerCbr::OnNack(interest);
}


void StatisticsConsumer::OnInterest (const Ptr<const Interest> interest)
{
    //NS_LOG_DEBUG ("Received Interest " << interest->GetName());
    ndn::ConsumerCbr::OnInterest(interest);
}

void StatisticsConsumer::OnTimeout(uint32_t sequenceNumber)
{
    nrTimeouts++;
    //NS_LOG_DEBUG ("Timeout for " << sequenceNumber);
    ndn::ConsumerCbr::OnTimeout (sequenceNumber);
}

void StatisticsConsumer::OnData(Ptr<const Data> contentObject)
{
    nrSatisfiedInterests++;
    //NS_LOG_DEBUG ("Received data for " << contentObject->GetName());
    ndn::ConsumerCbr::OnData (contentObject);
}

void StatisticsConsumer::StopApplication()
{
    //NS_LOG_DEBUG ("Sent interests: " << nrSentInterests << ", timeouts: " << nrTimeouts << ", NACKS: " << nrNacks);
    fprintf(stderr, "Sent = %d ; Statisfied = %d ; Nacks = %d ; Timeouts = %d\n", nrSentInterests, nrSatisfiedInterests, nrNacks, nrTimeouts);
    ndn::ConsumerCbr::StopApplication();
}

}
}

