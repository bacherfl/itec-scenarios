#include "forwardingengine.h"

using namespace ns3::ndn;

ForwardingEngine::ForwardingEngine(std::vector<Ptr<ndn::Face> > faces)
{
  init(faces);
}

void ForwardingEngine::init (std::vector<Ptr<ndn::Face> > faces)
{

  faceIds.clear ();
  clearForwardingPropabilityMap();

  faceIds.push_back (DROP_FACE_ID); // add dropping face

  for(std::vector<Ptr<ndn::Face> >::iterator it = faces.begin (); it != faces.end (); ++it)
  {
    faceIds.push_back ((*it)->GetId());
  }
}

int ForwardingEngine::determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest)
{

  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(interest->GetName());

  if(fwMap.find(prefix) == fwMap.end ())
  {
    fprintf(stderr, "1\n");
    fwMap[prefix] = new ForwardingProbabilityTable(faceIds);
  }

  return fwMap[prefix]->determineOutgoingFace(inFace, interest);

}

std::string ForwardingEngine::extractContentPrefix(ndn::Name name)
{
  return name.get(0).toUri ();
}

void ForwardingEngine::clearForwardingPropabilityMap()
{
  for(ForwardingPropabilityMap::iterator it = fwMap.begin (); it != fwMap.end (); ++it)
  {
    delete it->second;
  }

  fwMap.clear ();
}


