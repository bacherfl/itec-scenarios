#ifndef STATISTICSCONSUMER_H
#define STATISTICSCONSUMER_H

#endif // STATISTICSCONSUMER_H

#include "ns3/ndnSIM/apps/ndn-consumer.h"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.h"
#include "ns3/ndn-app.h"

namespace ns3 {
namespace ndn {

    class StatisticsConsumer : public ndn::ConsumerCbr
    {
    public:

        StatisticsConsumer ();
        virtual ~StatisticsConsumer ();

        //register NS-3 Type StatisticsConsumer
        static TypeId GetTypeId();

        virtual void
        OnTimeout (uint32_t sequenceNumber);

        virtual void
        OnNack (const Ptr<const Interest> &interest);

        virtual void
        OnData (Ptr<const Data> contentObject);

        virtual void
        WillSendOutInterest(uint32_t sequenceNumber);

        virtual void
        StopApplication();

    private:
        long nrSentInterests;
        long nrSatisfiedInterests;
        long nrTimeouts;
        long nrNacks;
    };
}
}

