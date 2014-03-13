#ifndef FACEPITSTATISTIC_H
#define FACEPITSTATISTIC_H


#include <ns3/log.h>
#include <ns3/ndn-face.h>


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

      protected:

       unsigned int pendingInterestCount;

      private:
       Ptr<ndn::Face> face;
      };
    }
  }
}

#endif // FACEPITSTATISTIC_H
