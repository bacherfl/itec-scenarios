#ifndef FORWARDINGSTATISTICS_H
#define FORWARDINGSTATISTICS_H

#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/log.h"
#include "ns3/ndn-pit.h"
#include "ns3/simple-ref-count.h"

#include <stdio.h>

#define UPDATE_INTERVALL 0.5

namespace ns3
{
namespace ndn
{
class ForwardingStatistics : public SimpleRefCount<ForwardingStatistics>
{
public:
  ForwardingStatistics(std::vector<int> faceIds);

  void logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry);
  void logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
  void logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace);

  double getLinkReliability(int face_id);
  double getGoodput(int face_id);
  double getUnstatisfiedTrafficFraction(){return unstatisfied_traffic_fraction;}

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

  void calculateLinkReliabilities();
  void calculateGoodput();
  void calculateUnstatisfiedTrafficFraction();

  double unstatisfied_traffic_fraction;
  ForwardingDoubleMap last_goodput;
  ForwardingDoubleMap last_reliability;

  ForwardingIntMap statisfied_requests;
  ForwardingIntMap unstatisfied_requests;
  ForwardingIntMap goodput_bytes_received;
};

}
}
#endif // FORWARDINGSTATISTICS_H
