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

      Ptr<NetDevice> nd = face->GetNode()->GetDevice(face->GetId());
      if (nd != NULL)
      {
          Ptr<PointToPointNetDevice> nd1 = nd->GetObject<PointToPointNetDevice>();
          DataRateValue dv;
          nd1->GetAttribute("DataRate", dv);
          DataRate d = dv.Get();
          uint64_t bitRate = d.GetBitRate();

          SDNController::SetLinkBitrate(face->GetNode()->GetId(), face->GetId(), bitRate);
      }

}

void SDNControlledStrategy::AssignBandwidth(const std::string &prefix, int faceId, uint64_t bitrate)
{
    qosQueues[faceId][prefix] = new ns3::ndn::utils::QoSQueue(bitrate);
}

void SDNControlledStrategy::PushRule(const std::string &prefix, int faceId)
{
    std::cout << "new rule: " << prefix << " -> face " << faceId << "\n";
    localFib[prefix].push_back(faceId);
    FlowEntry *fe = new FlowEntry;
    fe->bytesReceived = 0;
    fe->faceId = faceId;
    fe->receivedInterests = 0;
    fe->satisfiedInterests = 0;
    fe->unsatisfiedInterests = 0;
    flowTable[prefix] = fe;
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
    std::string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
    Ptr<Node> node = GetObject<Node>();
    //let the controller calculate the route and push the rules to all nodes on the path to the target
    SDNController::CalculateRoutesForPrefix(node->GetId(), prefix);

    return SelectFaceFromLocalFib(interest);
}


Ptr<Face> SDNControlledStrategy::SelectFaceFromLocalFib(Ptr<const Interest> interest)
{
    std::string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
    std::cout << prefix << "\n";
    if (flowTable[prefix] != NULL)
    {
        //flowTable[prefix]
        FlowEntry *fe = flowTable[prefix];
        int faceId = fe->faceId;
        fe->receivedInterests++;

        for (int i = 0; i < faces.size(); i++)
        {
            Ptr<Face> face = faces.at(i);
            std::cout << "FaceID = " << face->GetId() << "\n";
            if (face->GetId() == faceId)
            {
                Ptr<Node> node = this->GetObject<Node>();
                std::cout << node->GetId() << " forwarding interest via face " << faceId << "\n";
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

    if (outFace != NULL)
    {
        std::string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
        if (qosQueues[outFace->GetId()].size() > 0 && qosQueues[outFace->GetId()][prefix] != NULL)
        {
            if (qosQueues[outFace->GetId()][prefix]->TryForwardInterest())
            {
                if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                    propagatedCount++;
            }
        }

        else if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
            propagatedCount ++;
    }
    //we're on the target node where the prefix is available --> forward to app face
    else {

        Ptr<Node> node = this->GetObject<Node>();

        typedef fib::FaceMetricContainer::type::index<fib::i_metric>::type FacesByMetric;
        FacesByMetric &faces = pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ();
        FacesByMetric::iterator faceIterator = faces.begin ();

        int propagatedCount = 0;

        // forward to best-metric face
        if (faceIterator != faces.end ())
        {
            std::cout << node->GetId() << " forwarding interest to face " << faceIterator->GetFace()->GetId() << "\n";
            if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
                propagatedCount ++;

            faceIterator++;

        }
        // forward to second-best-metric face
        if (faceIterator != faces.end ())
        {
            std::cout << node->GetId() << " forwarding interest to face " << faceIterator->GetFace()->GetId() << "\n";
            if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
                propagatedCount ++;

            faceIterator ++;
        }

    }
    std::cout << "Propagated count: " << propagatedCount << "\n";

    return propagatedCount > 0;
}


void SDNControlledStrategy::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  ForwardingStrategy::WillEraseTimedOutPendingInterest(pitEntry);
}


void SDNControlledStrategy::WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  if(inFace != 0) // ==0 means data comes from cache
  {
      //fwEngine->logStatisfiedRequest(inFace,pitEntry);
      FlowEntry *fe = flowTable[pitEntry->GetInterest()->GetName().toUri()];
      if (fe != NULL) {
          fe->satisfiedInterests++;
      }
  }


  ForwardingStrategy::WillSatisfyPendingInterest(inFace,pitEntry);
}

void SDNControlledStrategy::OnData(Ptr<Face> face, Ptr<Data> data)
{
    ForwardingStrategy::OnData(face, data);
}

void SDNControlledStrategy::DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  //fprintf(stderr, "SendOut %s on Face %d\n", interest->GetName ().toUri().c_str(), outFace->GetId ());
  ForwardingStrategy::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}


void SDNControlledStrategy::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{   
    Ptr<Node> node = this->GetObject<Node>();
    std::string prefix = nack->GetName().toUri();
    FlowEntry *fe = flowTable[prefix];
    fe->unsatisfiedInterests++;

    //check if ratio of unsatisfied to satisfied requests exceeds some limit and tell the controller
    if (((double) fe->receivedInterests) / ((double) fe->unsatisfiedInterests) < MIN_SAT_RATIO )
    {
        Ptr<Node> node = this->GetObject<Node>();
        SDNController::LinkFailure(node->GetId(), fe->faceId, prefix);
    }


    //SDNController::DidReceiveValidNack(node->GetId(), inFace->GetId(), nack->GetName().toUri());
  //ForwardingStrategy::DidReceiveValidNack (inFace, nackCode, nack, pitEntry);
}

/*
bool SDNControlledStrategy::TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{

  if(useTockenBucket > 0)
  {
    if(!fwEngine->tryForwardInterest (outFace, interest))
      return false;
  }

  return ForwardingStrategy::TrySendOutInterest(inFace,outFace, interest, pitEntry);
}
*/


void SDNControlledStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  return ForwardingStrategy::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


