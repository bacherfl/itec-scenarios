#ifndef FORWARDINGENGINE_H
#define FORWARDINGENGINE_H

#include "ns3/simple-ref-count.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"

#include "boost/unordered_map.hpp"

#include "vector"
#include "stdio.h"

#include "forwardingentry.h"

namespace ns3
{
namespace ndn
{

class ForwardingEngine : public SimpleRefCount<ForwardingEngine>
{
public:
  ForwardingEngine(std::vector<Ptr<ndn::Face> > faces);

  ~ForwardingEngine();

  int determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest);
  void logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry);
  void logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
  void logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace);
  void logDroppingFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);

protected:

  void update();

  void init(std::vector<Ptr<ndn::Face> > faces);
  std::string extractContentPrefix(Name name);
  //void clearForwardingPropabilityMap();

  std::vector<int> faceIds;

  /* map for storing stats for all faces */
  typedef std::map
    < std::string, /*content-prefix*/
      Ptr<ForwardingEntry> /*forwarding prob. table*/
    > ForwardingEntryMap;

  ForwardingEntryMap fwMap;

  EventId updateEvent;

};

}
}
#endif // FORWARDINGENGINE_H
