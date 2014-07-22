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

namespace ns3
{
namespace ndn
{

class ForwardingEntry : public SimpleRefCount<ForwardingEntry>
{
public:
  ForwardingEntry(std::vector<int> faceIds);

  int determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest);

  void logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry);
  void logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
  void logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace);
  void logDroppingFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);

  void update();

private:
  Ptr<ForwardingProbabilityTable> fwTable;
  Ptr<ForwardingStatistics> fwStats;

  int determineContentLayer(Ptr<const Interest> interest);
};

}
}
#endif // FORWARDINGENTRY_H
