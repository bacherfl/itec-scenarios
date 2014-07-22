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

  fwTable->updateColumns(fwStats);
  fwTable->syncDroppingPolicy(fwStats);

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
  fwStats->logExhaustedFace(inFace,interest,pitEntry,targetedOutFace,determineContentLayer(pitEntry->GetInterest ()));
}

void ForwardingEntry::logDroppingFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  fwStats->logDroppingFace(inFace,interest,pitEntry,determineContentLayer(pitEntry->GetInterest ()));
}

/*
 *This function has to be adapted to fit your content needs....
 */
int ForwardingEntry::determineContentLayer(Ptr<const Interest> interest)
{  

   // extract level tag from packet
  SVCLevelTag levelTag;
  Ptr<Packet> packet = Wire::FromInterest (interest);
  bool svcLevelTagExists = packet->PeekPacketTag (levelTag);

  //TODO fix levels
  if (svcLevelTagExists)
  {
    return levelTag.Get ();
  }

  std::string layer = interest->GetName ().get(1).toUri();
  layer = layer.substr (layer.length ()-1, layer.length ());

  return boost::lexical_cast<int>( layer.c_str() );
}
