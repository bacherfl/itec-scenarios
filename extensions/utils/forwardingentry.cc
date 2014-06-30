#include "forwardingentry.h"

using namespace ns3::ndn;

ForwardingEntry::ForwardingEntry(std::vector<int> faceIds)
{
  this->fwTable = Create<ForwardingProbabilityTable>(faceIds);
  this->fwStats = Create<ForwardingStatistics>(faceIds);
}

void ForwardingEntry::update()
{
  fwStats->resetStatistics ();
  // todo fwTable Update
}
