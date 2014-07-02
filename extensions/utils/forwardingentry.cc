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

  //first update columns of table (layer specific updates)

}

int ForwardingEntry::determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest)
{
  return fwTable->determineOutgoingFace(inFace, interest, determineContentLayer(interest));
}

void ForwardingEntry::logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry)
{
  fwStats->logUnstatisfiedRequest(pitEntry,determineContentLayer(pitEntry->GetInterest ()));
}

void ForwardingEntry::logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  fwStats->logStatisfiedRequest(inFace,pitEntry,determineContentLayer(pitEntry->GetInterest()));
}

void ForwardingEntry::logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace)
{
  fwStats->logExhaustedFace(inFace,interest,pitEntry,targetedOutFace,determineContentLayer(pitEntry->GetInterest ())); //TODO PER LAYER
}

/*
 *This function has to be adapted to fit your content needs....
 */
int ForwardingEntry::determineContentLayer(Ptr<const Interest> interest)
{  
  std::string layer = interest->GetName ().get(1).toUri();
  layer = layer.substr (layer.length ()-1, layer.length ());

  return boost::lexical_cast<int>( layer.c_str() );
}
