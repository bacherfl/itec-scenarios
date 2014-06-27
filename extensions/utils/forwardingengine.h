#ifndef FORWARDINGENGINE_H
#define FORWARDINGENGINE_H

#include "ns3/simple-ref-count.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"

#include "boost/unordered_map.hpp"

#include "vector"
#include "stdio.h"

#include "forwardingprobabilitytable.h"

namespace ns3
{
namespace ndn
{

class ForwardingEngine : public SimpleRefCount<ForwardingEngine>
{
public:
  ForwardingEngine(std::vector<Ptr<ndn::Face> > faces);

  int determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest);

protected:
  void init(std::vector<Ptr<ndn::Face> > faces);
  std::string extractContentPrefix(Name name);
  void clearForwardingPropabilityMap();

  std::vector<int> faceIds;

  /* map for storing stats for all faces */
  typedef std::map
    < std::string, /*content-prefix*/
      ForwardingProbabilityTable* /*forwarding prob. table*/
    > ForwardingPropabilityMap;

  ForwardingPropabilityMap fwMap;

};

}
}
#endif // FORWARDINGENGINE_H
