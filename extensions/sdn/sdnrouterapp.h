#ifndef SDNROUTERAPP_H
#define SDNROUTERAPP_H

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

class SDNRouterApp : public ndn::App
{
    public:
        SDNRouterApp();
        virtual ~SDNRouterApp ();

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

        void DiscoverController();
        void DiscoverNeighbours();

        void RegisterAtController();
private:
        int64_t lastNeighbourUpdate;
        int64_t lastControllerResponse;
        int64_t lastControllerRegistrationSent;


};

} // namespace sdn
} //namespace sdn
} //namespace ndn

#endif // SDNROUTERAPP_H
