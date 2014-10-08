#ifndef SDNCONTROLLER_H
#define SDNCONTROLLER_H

#include "ns3/ndn-app.h"
#include "sdninteresthandler.h"

#include "ns3-dev/ns3/ndn-app-face.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-data.h"
#include "ns3-dev/ns3/ndn-fib.h"


#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace sdn {

class SDNController : public ndn::App
{
    public:
        SDNController();
        virtual ~SDNController ();

        static TypeId
        GetTypeId();

        virtual void
        StartApplication();

        virtual void
        StopApplication();

        virtual void
        OnInterest(Ptr<const Interest> interest);

        virtual void
        OnData(Ptr<const Data> contentObject);

        void SendNeighbourRequestToRouter(std::string routerId);
private:
        Ptr<SDNInterestHandler> sdnInterestHandler;
};

} // namespace sdn
} //namespace sdn
} //namespace ndn
#endif // SDNCONTROLLER_H
