#include "qosqueue.h"

namespace ns3 {
namespace ndn {
namespace utils {

QoSQueue::QoSQueue(uint64_t bitrate):
    bitrate(bitrate)
{
    tokenBucket = new TokenBucket(BUCKET_SIZE);
    double packets_per_sec = bitrate / 8 ;
    packets_per_sec /= (DATA_PACKET_SIZE + INTEREST_PACKET_SIZE);
    tokenGenRate = packets_per_sec / 1000; // tokens per ms
    tokenGenRate *= TOKEN_FILL_INTERVALL; // tokens per intervall

    //std::cout << "token gen rate = " << tokenGenRate << "\n";

    this->newTokenEvent = Simulator::Schedule(Seconds(0), &QoSQueue::newToken, this);
}

QoSQueue::~QoSQueue()
{
    Simulator::Cancel(this->newTokenEvent);
}

void  QoSQueue::newToken()
{
    double addedTokens = tokenBucket->addTokens(tokenGenRate);
    //std::cout << "added tokens: " << addedTokens << "\n";

    this->newTokenEvent = Simulator::Schedule(MilliSeconds(TOKEN_FILL_INTERVALL), &QoSQueue::newToken, this);
}

bool QoSQueue::TryForwardInterest()
{
    if (bitrate > 0)
        return tokenBucket->tryConsumeToken();
    else return true;
}

}
}
}

