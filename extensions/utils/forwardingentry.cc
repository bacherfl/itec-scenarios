#include "forwardingentry.h"

using namespace ns3::ndn;

NS_LOG_COMPONENT_DEFINE ("ForwardingEntry");

ForwardingEntry::ForwardingEntry(std::vector<int> faceIds, Ptr<fib::Entry> fibEntry)
{
  this->fibEntry = fibEntry;

  initFaceIds(faceIds);

  faceIds_active = faceIds;

  this->fwTable = Create<ForwardingProbabilityTable>(faceIds_active);
  this->fwStats = Create<ForwardingStatistics>(faceIds_active);
}

void ForwardingEntry::initFaceIds(std::vector<int> faceIds)
{
  if(fibEntry == NULL)
  {
    NS_LOG_DEBUG("Fib Entry is NULL adding all faces to FWT as Route is unknown!\n");
    faceIds_active = faceIds;
    return;
  }

  for(std::vector<int>::iterator it = faceIds.begin (); it != faceIds.end (); it++)
  {
    if(faceInRoutingInformation (*it))
    {
        faceIds_active.push_back (*it);
        NS_LOG_DEBUG("Pushed Face(" << *it << ") to ACTIVE Faces");
    }
    else
    {
      faceIds_standby.push_back (*it);
      NS_LOG_DEBUG("Pushed Face(" << *it << ") to STANDBY Faces");
    }
  }
}

void ForwardingEntry::update()
{

  fwStats->resetStatistics ();

  fwTable->updateColumns(fwStats);
  fwTable->syncDroppingPolicy(fwStats);

  //now check if we should remove / add faces

  checkForAddFaces();
  checkForRemoveFaces();

}

void ForwardingEntry::checkForAddFaces()
{
  if(faceIds_standby.empty ())
    return;

  //todo
}

void ForwardingEntry::checkForRemoveFaces()
{
  //first update map
  std::vector<int> faces_to_remove;
  for(std::vector<int>::iterator it = faceIds_active.begin(); it != faceIds_active.end(); ++it) // for each ur_face
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

    if( (tMap[*it] > FACE_REMOVE_CYCLES && !faceInRoutingInformation (*it)) )
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
    this->removeFace (faceIds_active,*it); /*from entry*/
    this->addFace (faceIds_standby,*it); /*from entry*/
  }

}

bool ForwardingEntry::faceInRoutingInformation(int faceId)
{
  for (ndn::fib::FaceMetricContainer::type::index<ndn::fib::i_face>::type::iterator metric = fibEntry->m_faces.get<ndn::fib::i_face> ().begin ();
       metric != fibEntry->m_faces.get<ndn::fib::i_face> ().end (); metric++)
  {
    if(faceId == metric->GetFace()->GetId() || faceId == DROP_FACE_ID)
    {
      return true;
    }
  }
  return false;
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

void ForwardingEntry::removeFace(std::vector<int> &from, int faceId)
{
  //remove from vector...
  for(std::vector<int>::iterator it = from.begin (); it != from.end (); ++it)
  {
    if(*it == faceId)
    {
      from.erase (it);
    std::sort(from.begin(), from.end());//order
    return;
    }
  }
  NS_LOG_UNCOND("Could not erase from ForwardingEntry vector: " << faceId);
}

void ForwardingEntry::addFace(std::vector<int> &to, int faceId)
{
  to.push_back (faceId);
  std::sort(to.begin(), to.end());//order
}
