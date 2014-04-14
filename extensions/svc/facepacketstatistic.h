#ifndef FACEPACKETSTATISTIC_H
#define FACEPACKETSTATISTIC_H

#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/network-module.h"
#include <ns3/log.h>
#include <ns3/ndn-face.h>

#include "../droppingPolicy/packetbaseddroppingpolicy.h"
#include "../droppingPolicy/smoothlevelstatistics.h"


namespace ns3
{
  namespace ndn
  {
    namespace svc
    {
      class FacePacketStatistic
      {
      public:
        FacePacketStatistic();
        ~FacePacketStatistic();
        LevelStatistics* getStats();
        DroppingPolicy*  getPolicy();

        void SetPacketsPerTime(unsigned int packets);
        unsigned int GetPacketsPerTime();

        void IncreasePackets(unsigned int level);
        double GetDropProbability(unsigned int level);


        void UpdatePolicy(double metric);

      protected:
        unsigned int packets_per_time;
        LevelStatistics* stats;
        DroppingPolicy* policy;

      };
    }
  }
}

#endif // FACEPACKETSTATISTIC_H
