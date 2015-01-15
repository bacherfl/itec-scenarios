#include "sdncontrolledstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (SDNControlledStrategy);

LogComponent SDNControlledStrategy::g_log = LogComponent (SDNControlledStrategy::GetLogName ().c_str ());


SDNControlledStrategy::SDNControlledStrategy():
    initialized(false)
{

}

void SDNControlledStrategy::init()
{
    Ptr<Node> node = this->GetObject<Node>();
    SDNController::registerForwarder(this, node->GetId());
    initialized = true;
}

TypeId SDNControlledStrategy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::SDNControlledStrategy")
      .SetGroupName ("Ndn")
      .SetParent <ForwardingStrategy> ()
      .AddConstructor <SDNControlledStrategy> ();
  return tid;
}


std::string SDNControlledStrategy::GetLogName ()
{
  return ForwardingStrategy::GetLogName () + ".SDNControlledStrategy";
}

/* add face */

void SDNControlledStrategy::AddFace (Ptr<Face> face)
{
    if (!initialized)
        this->init();

  // add face to faces vector
    /*
  Ptr<Node> node = this->GetObject<Node> ();
  Ptr<Node> targetNode = face->GetNode();
  SDNController::AddLink(node, targetNode, face->GetId());
  */
  faces.push_back (face);
  //fwEngine = new utils::ForwardingEngine(faces, SDNControlledStrategy::m_fib, prefixComponentNum);
  ForwardingStrategy::AddFace(face);
}

void SDNControlledStrategy::PushRule(const std::string &prefix, int faceId)
{
    std::cout << "new rule: " << prefix << " -> face " << faceId << "\n";
    localFib[prefix].push_back(faceId);
}

/* remove face */

void SDNControlledStrategy::RemoveFace (Ptr<Face> face)
{
  // remove face from faces vector
  for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    if (face->GetId ()== (*it)->GetId())
    {
      faces.erase (it);
      break;
    }
  }

  //fwEngine = new utils::ForwardingEngine(faces, SDNControlledStrategy::m_fib, prefixComponentNum);
  ForwardingStrategy::RemoveFace(face);
}


void SDNControlledStrategy::OnInterest (Ptr< Face > inFace, Ptr< Interest > interest)
{
  ForwardingStrategy::OnInterest(inFace,interest);

  /*if(interest->GetNack () == Interest::NORMAL_INTEREST)
  {
    ForwardingStrategy::OnInterest(inFace,interest);
  }
  else
  {
    Ptr<pit::Entry> pitEntry = SDNControlledStrategy::m_pit->Lookup (*interest);
    if (pitEntry == 0)
    {
      SDNControlledStrategy::m_dropNacks (interest, inFace);
      return;
    }
    fwEngine->logUnstatisfiedRequest (pitEntry);
    // we dont call ForwardingStrategy::NACK(), since we skip looking for other sources.

    // set all outgoing faces to useless (in vain)
    for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
        it !=  faces.end (); ++it)
    {
        if ((*it)->GetId() != inFace->GetId())
        {
          pitEntry->AddOutgoing ((*it));
          pitEntry->SetWaitingInVain ((*it));
        }
    }

    //forward nack
    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (interest->GetNack ());
    BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      incoming.m_face->SendInterest (nack);
      SDNControlledStrategy::m_outNacks (nack, incoming.m_face);
    }

    pitEntry->RemoveIncoming (inFace);
    pitEntry->ClearOutgoing ();
  }*/

}


Ptr<Interest> SDNControlledStrategy::prepareNack(Ptr<const Interest> interest)
{
  Ptr<Interest> nack = Create<Interest> (*interest);

  //nack->SetNack (ndn::Interest::NACK_CONGESTION);
  nack->SetNack (ndn::Interest::NACK_CONGESTION); // set this since ndn changes it anyway to this.

  /*SVCLevelTag levelTag; // this causes loops together with limits on the clients be careful
  levelTag.Set (-1); // means packet dropped on purpose
  nack->GetPayload ()->AddPacketTag (levelTag);*/

  //fprintf(stderr, "NACK %s prepared at time: %f\n", interest->GetName ().toUri ().c_str (), Simulator::Now ().ToDouble (Time::S));
  return nack;
}


Ptr<Face> SDNControlledStrategy::GetFaceFromSDNController(Ptr<const Interest> interest)
{
    std::string prefix = interest->GetName().toUri();
    Ptr<Node> node = GetObject<Node>();
    //let the controller calculate the route and push the rules to all nodes on the path to the target
    SDNController::CalculateRoutesForPrefix(node->GetId(), prefix);

    return SelectFaceFromLocalFib(interest);
}


Ptr<Face> SDNControlledStrategy::SelectFaceFromLocalFib(Ptr<const Interest> interest)
{
    std::string prefix = interest->GetName().toUri();
    std::cout << prefix << "\n";
    if (localFib[prefix].size() > 0)
    {
        int faceId = localFib[prefix].at(0);

        for (int i = 0; i < faces.size(); i++)
        {
            Ptr<Face> face = faces.at(i);
            std::cout << "FaceID = " << face->GetId() << "\n";
            if (face->GetId() == faceId)
            {
                std::cout << "forwarding interest via face " << faceId << "\n";
                return face;
            }
        }
    }
    std::cout << "no face found in local fib \n";
    return NULL;
}


bool SDNControlledStrategy::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    Ptr<Face> outFace = SelectFaceFromLocalFib(interest);

    int propagatedCount = 0;

    if (outFace == NULL)
    {
        outFace = GetFaceFromSDNController(interest);
    }

    if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
        propagatedCount ++;

    /*

    typedef fib::FaceMetricContainer::type::index<fib::i_metric>::type FacesByMetric;
      FacesByMetric &faces = pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ();
      FacesByMetric::iterator faceIterator = faces.begin ();

      int propagatedCount = 0;

      // forward to best-metric face
      if (faceIterator != faces.end ())
        {          
          if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
            propagatedCount ++;

          faceIterator ++;
        }

      // forward to second-best-metric face
      if (faceIterator != faces.end ())
        {
          if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
            propagatedCount ++;

          faceIterator ++;
        }
      */
      return propagatedCount > 0;
}


void SDNControlledStrategy::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  ForwardingStrategy::WillEraseTimedOutPendingInterest(pitEntry);
}


void SDNControlledStrategy::WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  if(inFace != 0) // ==0 means data comes from cache
    fwEngine->logStatisfiedRequest(inFace,pitEntry);

  ForwardingStrategy::WillSatisfyPendingInterest(inFace,pitEntry);
}


void SDNControlledStrategy::DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  //fprintf(stderr, "SendOut %s on Face %d\n", interest->GetName ().toUri().c_str(), outFace->GetId ());
  ForwardingStrategy::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}


void SDNControlledStrategy::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{
  fwEngine->logUnstatisfiedRequest (inFace, pitEntry);
  //ForwardingStrategy::DidReceiveValidNack (inFace, nackCode, nack, pitEntry);
}


bool SDNControlledStrategy::TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
    /*
  if(useTockenBucket > 0)
  {
    if(!fwEngine->tryForwardInterest (outFace, interest))
      return false;
  }
  */
  return ForwardingStrategy::TrySendOutInterest(inFace,outFace, interest, pitEntry);
}


void SDNControlledStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  return ForwardingStrategy::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


