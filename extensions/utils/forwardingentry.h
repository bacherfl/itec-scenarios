#ifndef FORWARDINGENTRY_H
#define FORWARDINGENTRY_H

#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/log.h"
#include "ns3/simple-ref-count.h"

#include <vector>
#include <stdio.h>

#include "forwardingprobabilitytable.h"
#include "forwardingstatistics.h"
namespace ns3
{
namespace ndn
{

class ForwardingEntry : public SimpleRefCount<ForwardingEntry>
{
public:
  ForwardingEntry(std::vector<int> faceIds);

  Ptr<ForwardingProbabilityTable> getFWTable(){return fwTable;}
  Ptr<ForwardingStatistics> getFWStats(){return fwStats;}

  void update();

private:
  Ptr<ForwardingProbabilityTable> fwTable;
  Ptr<ForwardingStatistics> fwStats;
};

}
}
#endif // FORWARDINGENTRY_H
