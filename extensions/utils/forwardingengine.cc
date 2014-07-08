#include "forwardingengine.h"

using namespace ns3::ndn;

ForwardingEngine::ForwardingEngine(std::vector<Ptr<ndn::Face> > faces)
{
  init(faces);
  Simulator::Schedule(Seconds(UPDATE_INTERVALL), &ForwardingEngine::update, this);
}

void ForwardingEngine::init (std::vector<Ptr<ndn::Face> > faces)
{

  faceIds.clear ();
  //clearForwardingPropabilityMap(); not needed we have smart pointers now

  faceIds.push_back (DROP_FACE_ID); // add dropping face

  for(std::vector<Ptr<ndn::Face> >::iterator it = faces.begin (); it != faces.end (); ++it)
  {
    faceIds.push_back ((*it)->GetId());
  }

  std::sort(faceIds.begin(), faceIds.end()); // order faces strictly by ID
}

int ForwardingEngine::determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest)
{

  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(interest->GetName());

  if(fwMap.find(prefix) == fwMap.end ())
  {
    fwMap[prefix] = Create<ForwardingEntry>(faceIds);
  }

  Ptr<ForwardingEntry> entry = fwMap.find(prefix)->second;

  int out_face_id = entry->determineRoute(inFace, interest);

  return out_face_id;

}

std::string ForwardingEngine::extractContentPrefix(ndn::Name name)
{
  return name.get(0).toUri ();
}

void ForwardingEngine::logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry)
{
  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(pitEntry->GetInterest()->GetName());
  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logUnstatisfiedRequest");
  }

  fwMap[prefix]->logUnstatisfiedRequest(pitEntry);
}

void ForwardingEngine::logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  std::string prefix = extractContentPrefix(pitEntry->GetInterest()->GetName());
  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logStatisfiedRequest");
  }

   fwMap[prefix]->logStatisfiedRequest(inFace,pitEntry);
}

void ForwardingEngine::logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace)
{
  std::string prefix = extractContentPrefix(pitEntry->GetInterest()->GetName());
  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logExhaustedFace");
  }

   fwMap[prefix]->logExhaustedFace(inFace,interest,pitEntry,targetedOutFace);
}

void ForwardingEngine::update ()
{

  for(ForwardingEntryMap::iterator it = fwMap.begin (); it != fwMap.end (); ++it)
  {
    it->second->update();
  }

  Simulator::Schedule(Seconds(UPDATE_INTERVALL), &ForwardingEngine::update, this);
}

void ForwardingEngine::logDroppingFace (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(interest->GetName());
  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logDroppingFace");
  }

  fwMap[prefix]->logDroppingFace(inFace,interest,pitEntry);
}

/*void ForwardingEngine::clearForwardingPropabilityMap()
{
  for(ForwardingEntryMap::iterator it = fwMap.begin (); it != fwMap.end (); ++it)
  {
    delete it->second;
  }

  fwMap.clear ();
}*/


