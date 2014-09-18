#include "forwardingentry.h"

using namespace ns3::ndn;

NS_LOG_COMPONENT_DEFINE ("ForwardingEntry");

ForwardingEntry::ForwardingEntry(std::vector<int> faceIds)
{
  this->faceIds = faceIds;
  this->fwTable = Create<ForwardingProbabilityTable>(faceIds);
  this->fwStats = Create<ForwardingStatistics>(faceIds);
}

void ForwardingEntry::update()
{

  fwStats->resetStatistics ();

  fwTable->updateColumns(fwStats);
  fwTable->syncDroppingPolicy(fwStats);

  //now check if we should remove / add faces
  //checkFaces();

}

void ForwardingEntry::checkFaces()
{
  //first update map
  std::vector<int> faces_to_remove;
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each ur_face
  {
    if(*it == DROP_FACE_ID)
      continue;

    double min_usage = 1.0;
    double usage = 0.0;
    for(int layer = 0; layer < MAX_LAYERS; layer++)
    {
      usage = fwStats->getLinkReliability (*it, layer) * fwTable->getForwardingProbability (*it, layer);
      if(usage < min_usage)
        min_usage = usage;
    }

    if(min_usage < FACE_REMOVE_THRESHOLD)
      tMap[*it]++;
    else if(tMap[*it] > 0)
      tMap[*it]--;

    if(tMap[*it] > FACE_REMOVE_CYCLES)
    {
      //remove face
      faces_to_remove.push_back (*it);
      NS_LOG_UNCOND("Removed Face(" << *it << ") from FTW!");
    }
  }

  for(std::vector<int>::iterator it = faces_to_remove.begin (); it != faces_to_remove.end (); it++)
  {
    fwTable->removeFace (*it); /*from table*/
    fwStats->removeFace (*it); /*from stats*/
    this->removeFace (*it); /*from entry*/ //TODO later shift this to standby faces...
  }

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

void ForwardingEntry::removeFace(int faceId)
{
  //remove from vector...
  for(std::vector<int>::iterator it = faceIds.begin (); it != faceIds.end (); ++it)
  {
    if(*it == faceId)
    {
      faceIds.erase (it);
    std::sort(this->faceIds.begin(), this->faceIds.end());//order
    return;
    }
  }
  NS_LOG_UNCOND("Could not erase from ForwardingEntry vector: " << faceId);
}
