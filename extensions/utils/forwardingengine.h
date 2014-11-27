#ifndef FORWARDINGENGINE_H
#define FORWARDINGENGINE_H

#include "ns3/simple-ref-count.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"

#include "vector"
#include "stdio.h"

#include "forwardingentry.h"
#include "facebucketmanager.h"

namespace ns3
{
namespace ndn
{
namespace utils
{

class ForwardingEngine : public SimpleRefCount<ForwardingEngine>
{
public:
  ForwardingEngine(std::vector<Ptr<ndn::Face> > faces, Ptr<ndn::Fib> fib, unsigned int prefixComponentNumber);

  ~ForwardingEngine();

  bool tryForwardInterest(Ptr< Face > outFace, Ptr< const Interest > interest);

  int determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pit_entry, std::vector<int> blocked_faces);
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
  Ptr<ndn::Fib> fib;

  /* map for storing forwarding stats for all faces */
  typedef std::map
    < std::string, /*content-prefix*/
      Ptr<ForwardingEntry> /*forwarding prob. table*/
    > ForwardingEntryMap;

  ForwardingEntryMap fwMap;

  /* map for storing forwarding stats for all faces */
  typedef std::map
    < int, /*face ID*/
      Ptr<FaceBucketManager> /*face bucket manager*/
    > FaceBucketMap;

  FaceBucketMap fbMap;

  EventId updateEventFWT;

  unsigned int prefixComponentNumber;

};

}
}
}
#endif // FORWARDINGENGINE_H
