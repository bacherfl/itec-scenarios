#include "forwardingengine.h"

using namespace ns3::ndn::utils;

NS_LOG_COMPONENT_DEFINE ("ForwardingEngine");

ForwardingEngine::ForwardingEngine(std::vector<Ptr<ndn::Face> > faces,unsigned int prefixComponentNumber)
{
   this->prefixComponentNumber = prefixComponentNumber;
  init(faces);
  updateEvent = Simulator::Schedule(Seconds(UPDATE_INTERVALL), &ForwardingEngine::update, this);
}

ForwardingEngine::~ForwardingEngine ()
{
}

void ForwardingEngine::init (std::vector<Ptr<ndn::Face> > faces)
{

  fprintf(stderr, "init\n");
  faceIds.clear ();
  fbMap.clear ();

  faceIds.push_back (DROP_FACE_ID); // add dropping face

  for(std::vector<Ptr<ndn::Face> >::iterator it = faces.begin (); it != faces.end (); ++it)
  {
    fprintf(stderr,"adding face %d\n", (*it)->GetId());

    faceIds.push_back ((*it)->GetId());

    if(ndn::Face::APPLICATION & (*it)->GetFlags() == 0) // app face(s) dont get a buckt
    {
      fbMap[(*it)->GetId()] = Create<FaceBucketManager>(*it);
    }
  }

  std::sort(faceIds.begin(), faceIds.end()); // order faces strictly by ID
}

int ForwardingEngine::determineRoute(Ptr<Face> inFace, Ptr<const Interest> interest, bool &content_seen)
{

  content_seen = true;

  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(interest->GetName());

  if(fwMap.find(prefix) == fwMap.end ())
  {
    fwMap[prefix] = Create<ForwardingEntry>(faceIds);
    content_seen = false;

    // add buckets for all faces
    for(FaceBucketMap::iterator it = fbMap.begin (); it != fbMap.end (); it++)
    {
      it->second->addNewBucket(prefix);
    }
  }

  Ptr<ForwardingEntry> entry = fwMap.find(prefix)->second;

  int out_face_id = entry->determineRoute(inFace, interest);

  return out_face_id;

}

std::string ForwardingEngine::extractContentPrefix(ndn::Name name)
{
  return name.get(prefixComponentNumber).toUri ();
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
  NS_LOG_DEBUG("New FWT UPDATE at SimTime " << Simulator::Now ().GetSeconds () << "\n");

  /*experimental*/ //somthing is increasing the ref count from 1 to 2. i have no idee who...
  if(this->GetReferenceCount () == 1)
  {
      Simulator::Cancel (this->updateEvent);
      this->Unref ();
      return;
  }

  for(ForwardingEntryMap::iterator it = fwMap.begin (); it != fwMap.end (); ++it)
  {
    NS_LOG_DEBUG("Update for FWT for content: " << it->first.c_str());
    it->second->update();
  }

  updateEvent = Simulator::Schedule(Seconds(UPDATE_INTERVALL), &ForwardingEngine::update, this);
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

bool ForwardingEngine::tryForwardInterest(Ptr< Face > outFace, Ptr< const Interest > interest)
{
  std::string prefix = extractContentPrefix(interest->GetName());

  if(ndn::Face::APPLICATION & outFace->GetFlags() != 0) // app face(s) are not limited...
  {
    return true;
  }
  return fbMap[outFace->GetId ()]->tryForwardInterest(prefix);
}
