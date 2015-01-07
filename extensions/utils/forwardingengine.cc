#include "forwardingengine.h"

using namespace ns3::ndn::utils;

NS_LOG_COMPONENT_DEFINE ("ForwardingEngine");

ForwardingEngine::ForwardingEngine(std::vector<Ptr<ndn::Face> > faces, Ptr<ndn::Fib> fib, unsigned int prefixComponentNumber)
{
  this->prefixComponentNumber = prefixComponentNumber;
  this->fib = fib;
  init(faces);
  updateEventFWT = Simulator::Schedule(Seconds(ParameterConfiguration::getInstance ()->getParameter ("UPDATE_INTERVALL")), &ForwardingEngine::update, this);
}

ForwardingEngine::~ForwardingEngine ()
{
}

void ForwardingEngine::init (std::vector<Ptr<ndn::Face> > faces)
{

  //store node reference
  if(faces.size ()>0)
    node = faces.at(0)->GetNode();

  faceIds.clear ();
  fbMap.clear ();

  faceIds.push_back (DROP_FACE_ID); // add dropping face

  for(std::vector<Ptr<ndn::Face> >::iterator it = faces.begin (); it != faces.end (); ++it)
  {
    //fprintf(stderr,"adding face %d\n", (*it)->GetId());

    faceIds.push_back ((*it)->GetId());

    if(ndn::Face::APPLICATION & (*it)->GetFlags() == 0) // app face(s) dont get a buckt
    {
      fbMap[(*it)->GetId()] = Create<FaceBucketManager>(*it);
    }
  }

  std::sort(faceIds.begin(), faceIds.end()); // order faces strictly by ID
}

int ForwardingEngine::determineRoute(std::vector<Ptr<Face> > inFaces, Ptr<const Interest> interest, Ptr<pit::Entry> pit_entry, std::vector<int> blocked_faces)
{
  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(interest->GetName());

  if(fwMap.find(prefix) == fwMap.end ())
  {
    //fprintf(stderr, "prefix = %s\n", prefix.c_str ());
    //fwMap[prefix] = Create<ForwardingEntry>(faceIds, fib->Find(ndn::Name(prefix)));
    fwMap[prefix] = Create<ForwardingEntry>(faceIds, fib->LongestPrefixMatch (interest.operator * ()));

    // add buckets for all faces
    for(FaceBucketMap::iterator it = fbMap.begin (); it != fbMap.end (); it++)
    {
      it->second->addNewBucket(prefix);
    }
  }

  Ptr<ForwardingEntry> entry = fwMap.find(prefix)->second;

  int out_face_id = entry->determineRoute(inFaces, interest, blocked_faces);

  return out_face_id;

}

std::string ForwardingEngine::extractContentPrefix(ndn::Name name)
{
  std::string prefix ="";
  //fprintf(stderr, "number =%d\n", prefixComponentNumber);
  for(int i=0; i <= prefixComponentNumber; i++)
  {
    prefix.append ("/");
    prefix.append (name.get (i).toUri ());
  }
  return prefix;
}

void ForwardingEngine::logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry)
{
  for (pit::Entry::out_container::iterator face = pitEntry->GetOutgoing ().begin ();face != pitEntry->GetOutgoing ().end (); face ++)
    logUnstatisfiedRequest((*face).m_face, pitEntry);
}

void ForwardingEngine::logUnstatisfiedRequest(Ptr<Face> face, Ptr<pit::Entry> pitEntry)
{
  //check if content prefix has been seen
  std::string prefix = extractContentPrefix(pitEntry->GetInterest()->GetName());

  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logUnstatisfiedRequest");
    return;
  }

  fwMap[prefix]->logUnstatisfiedRequest(face, pitEntry);
}

void ForwardingEngine::logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  std::string prefix = extractContentPrefix(pitEntry->GetInterest()->GetName());
  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logStatisfiedRequest");
    return;
  }

   fwMap[prefix]->logStatisfiedRequest(inFace,pitEntry);
}

void ForwardingEngine::logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace)
{
  std::string prefix = extractContentPrefix(pitEntry->GetInterest()->GetName());
  if(fwMap.find(prefix) == fwMap.end ())
  {
    NS_LOG_UNCOND("Error in logExhaustedFace");
    return;
  }

   fwMap[prefix]->logExhaustedFace(inFace,interest,pitEntry,targetedOutFace);
}

void ForwardingEngine::update ()
{
  NS_LOG_DEBUG("FWT UPDATE at SimTime " << Simulator::Now ().GetSeconds () << " for node: '" <<   Names::FindName(node) << "'\n");

  /*experimental*/ //somthing is increasing the ref count from 1 to 2. i have no idea who...
  if(this->GetReferenceCount () == 1)
  {
      Simulator::Cancel (this->updateEventFWT);
      this->Unref ();
      return;
  }

  for(ForwardingEntryMap::iterator it = fwMap.begin (); it != fwMap.end (); ++it)
  {
    NS_LOG_DEBUG("Update for FWT for content: " << it->first.c_str() << "\n");
    it->second->update();
  }

  updateEventFWT = Simulator::Schedule(Seconds(ParameterConfiguration::getInstance ()->getParameter ("UPDATE_INTERVALL")), &ForwardingEngine::update, this);
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
