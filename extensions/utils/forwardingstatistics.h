#ifndef FORWARDINGSTATISTICS_H
#define FORWARDINGSTATISTICS_H

#include "forwardingprobabilitytable.h"

#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/log.h"
#include "ns3/ndn-pit.h"
#include "ns3/simple-ref-count.h"

#include <stdio.h>

#define UPDATE_INTERVALL 1.0

namespace ns3
{
namespace ndn
{
class ForwardingStatistics : public SimpleRefCount<ForwardingStatistics>
{
public:
  ForwardingStatistics(std::vector<int> faceIds);

  void logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry, int ilayer);
  void logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry, int ilayer);
  void logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace, int ilayer);

  double getLinkReliability(int face_id, int layer);
  double getGoodput(int face_id, int layer);
  double getUnstatisfiedTrafficFraction(int ilayer){return stats[ilayer].unstatisfied_traffic_fraction;}

  void resetStatistics();

protected:

  std::vector<int> faceIds;

  typedef std::map
    < int, /*face id*/
     int /*value to store*/
    > ForwardingIntMap;

  typedef std::map
    < int, /*face id*/
     double /*value to store*/
    > ForwardingDoubleMap;

  int getMapIndexFromFaceID(int face_id);

  void calculateLinkReliabilities(int layer);
  void calculateGoodput(int layer);
  void calculateUnstatisfiedTrafficFraction(int layer);

  struct ForwardingLayerStats
  {
    double unstatisfied_traffic_fraction;
    ForwardingDoubleMap last_goodput;
    ForwardingDoubleMap last_reliability;

    ForwardingIntMap statisfied_requests;
    ForwardingIntMap unstatisfied_requests;
    ForwardingIntMap goodput_bytes_received;

    ForwardingLayerStats()
    {
      unstatisfied_traffic_fraction = 0.0;
    }

    ForwardingLayerStats(const ForwardingLayerStats& other)
    {
      unstatisfied_traffic_fraction = other.unstatisfied_traffic_fraction;
      last_goodput = other.last_goodput;
      last_reliability = other.last_reliability;
      statisfied_requests = other.statisfied_requests;
      unstatisfied_requests = other.unstatisfied_requests;
      goodput_bytes_received = other.goodput_bytes_received;
    }

  };

  typedef std::map
  <int, /*content layer*/
  ForwardingLayerStats
  >ForwardingStatsMap;

  ForwardingStatsMap stats;
};

}
}
#endif // FORWARDINGSTATISTICS_H
