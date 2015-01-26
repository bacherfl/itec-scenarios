#ifndef QOSQUEUE_H
#define QOSQUEUE_H

#include "ns3/simple-ref-count.h"

#include "ns3/log.h"

#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/network-module.h"

#include "tokenbucket.h"
#include <string>
#include "ns3/ndn-face.h"

#define DATA_PACKET_SIZE 4096
#define INTEREST_PACKET_SIZE 50
#define TOKEN_FILL_INTERVALL 10 //ms
#define BUCKET_SIZE 100.0

namespace ns3 {
namespace ndn {
namespace utils {

class QoSQueue
{
public:
    QoSQueue(uint64_t bitrate);
    ~QoSQueue();
    bool TryForwardInterest();

private:
    TokenBucket *tokenBucket;
    double tokenGenRate;

    EventId newTokenEvent;

    void  newToken();
    uint64_t bitrate;
};

}
}
}

#endif // QOSQUEUE_H
