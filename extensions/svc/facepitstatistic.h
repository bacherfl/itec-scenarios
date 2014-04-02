#ifndef FACEPITSTATISTIC_H
#define FACEPITSTATISTIC_H

#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/network-module.h"
#include <ns3/log.h>
#include <ns3/ndn-face.h>

#define AVERAGE_DATAPACKET_SIZE 4200

namespace ns3
{
  namespace ndn
  {
    namespace svc
    {
      class FacePITStatistic
      {
      public:
       FacePITStatistic(Ptr<ndn::Face> face);

       void increase();
       void decrease();

       unsigned int getPendingInterestCount();
       unsigned int getResidualBandwidth();

      protected:

       unsigned int pendingInterestCount;

      private:
       Ptr<ndn::Face> face;
      };
    }
  }
}

#endif // FACEPITSTATISTIC_H
