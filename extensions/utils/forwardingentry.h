#ifndef FORWARDINGENTRY_H
#define FORWARDINGENTRY_H

#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/log.h"
#include "ns3/simple-ref-count.h"

#include <boost/lexical_cast.hpp>

#include <vector>
#include <stdio.h>

#include "forwardingprobabilitytable.h"
#include "forwardingstatistics.h"

#include "../svc/svcleveltag.h"
#include "ns3/ndn-wire.h"

#define FACE_REMOVE_THRESHOLD 0.01
#define FACE_REMOVE_CYCLES 50

namespace ns3
{
namespace ndn
{

class ForwardingEntry : public SimpleRefCount<ForwardingEntry>
{
public:
  ForwardingEntry(std::vector<int> faceIds, Ptr<fib::Entry> fibEntry);

  int determineRoute(std::vector<Ptr<Face> > inFaces, Ptr<const Interest> interest, std::vector<int> blocked_faces);

  void logUnstatisfiedRequest(Ptr<Face> face, Ptr<pit::Entry> pitEntry);
  void logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
  void logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace);
  void logDroppingFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);

  void update();

private:
  Ptr<ForwardingProbabilityTable> fwTable;
  Ptr<ForwardingStatistics> fwStats;
  std::vector<int> faceIds_active;
  std::vector<int> faceIds_standby;

  Ptr<fib::Entry> fibEntry;

  typedef std::map
    < int, /*face-id*/
      int /*number of cycles face forwarding prob below FACE_REMOVE_THRESHOLD*/
    > ThresholdCycleMap;

  ThresholdCycleMap tMap;

  void checkForRemoveFaces();
  void checkForAddFaces();

  bool evaluateFallback();

  bool faceInRoutingInformation(int faceId);

  void removeFace(std::vector<int> &from, int faceId);
  void addFace(std::vector<int> &to, int faceId);

  int determineContentLayer(Ptr<const Interest> interest);

  void initFaceIds(std::vector<int> faceIds);

  int fallbackCounter;
};

}
}
#endif // FORWARDINGENTRY_H

