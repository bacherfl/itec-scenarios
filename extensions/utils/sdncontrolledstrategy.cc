#include "sdncontrolledstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (SDNControlledStrategy);

LogComponent SDNControlledStrategy::g_log = LogComponent (SDNControlledStrategy::GetLogName ().c_str ());
const double SDNControlledStrategy::MIN_SAT_RATIO = 0.7;
const int SDNControlledStrategy::FACE_STATUS_GREEN = 0;
const int SDNControlledStrategy::FACE_STATUS_YELLOW = 1;
const int SDNControlledStrategy::FACE_STATUS_RED = 2;

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
      faces.push_back (face);
      ForwardingStrategy::AddFace(face);
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
    fe->status = FACE_STATUS_GREEN;
    flowTable[prefix].push_back(fe);
}

void SDNControlledStrategy::RemoveFace (Ptr<Face> face)
{
    for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
         it !=  faces.end (); ++it)
    {
        if (face->GetId ()== (*it)->GetId())
        {
            faces.erase (it);
            break;
        }
    }
    ForwardingStrategy::RemoveFace(face);
}


void SDNControlledStrategy::OnInterest (Ptr< Face > inFace, Ptr< Interest > interest)
{
    ForwardingStrategy::OnInterest(inFace,interest);
}


Ptr<Interest> SDNControlledStrategy::prepareNack(Ptr<const Interest> interest)
{
    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (ndn::Interest::NACK_CONGESTION); // set this since ndn changes it anyway to this.
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
    if (flowTable[prefix].size() > 0)
    {
        int idx = rand() % flowTable[prefix].size();
        //flowTable[prefix]
        FlowEntry *fe = flowTable[prefix].at(idx);
        int faceId = fe->faceId;

        fe->receivedInterests++;

        if (fe->receivedInterests >= 1000)
        {
            mtx_.lock();
            fe->receivedInterests = 0;
            fe->satisfiedInterests = 0;
            fe->unsatisfiedInterests = 0;
            mtx_.unlock();
        }

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


void SDNControlledStrategy::LogDroppedInterest(std::string prefix, Ptr<Face> face)
{
    std::vector<FlowEntry* > flowEntries = flowTable[prefix];

    for (std::vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == face->GetId())
        {
            mtx_.lock();
            fe->unsatisfiedInterests++;
            //check if ratio of unsatisfied to satisfied requests exceeds some limit and tell the controller
            double successRate = (double) fe->satisfiedInterests / (double) (fe->satisfiedInterests + fe->unsatisfiedInterests);
            mtx_.unlock();
            if (successRate < MIN_SAT_RATIO && fe->status == FACE_STATUS_GREEN)
            {
                fe->status = FACE_STATUS_RED;
                Ptr<Node> node = this->GetObject<Node>();
                SDNController::LinkFailure(node->GetId(), fe->faceId, prefix, 1 - successRate);
            }
        }
    }
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
            else {
                LogDroppedInterest(prefix, outFace);
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

        // forward to best-metric face
        if (faceIterator != faces.end ())
        {
            std::cout << node->GetId() << " forwarding interest to face " << faceIterator->GetFace()->GetId() << "\n";
            if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
                propagatedCount ++;

            faceIterator++;

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
        Name name = pitEntry->GetInterest()->GetName();
        std::string prefix = name.getPrefix(name.size() - 1).toUri();
        //fwEngine->logStatisfiedRequest(inFace,pitEntry);
        std::vector<FlowEntry* > flowEntries = flowTable[prefix];

        for (std::vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
        {
            FlowEntry *fe = (*it);
            if (fe->faceId == inFace->GetId())
            {

                mtx_.lock();
                fe->satisfiedInterests++;
                double successRate = (double) fe->satisfiedInterests / (double) (fe->satisfiedInterests + fe->unsatisfiedInterests);
                mtx_.unlock();
                std::cout << "satisfied: " << successRate << "\n";
                if ((fe->status == FACE_STATUS_RED) && (successRate > MIN_SAT_RATIO))
                {
                    Ptr<Node> node = this->GetObject<Node>();
                    SDNController::LinkRecovered(node->GetId(), inFace->GetId(), prefix, 1 - successRate);
                }
            }
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
    ForwardingStrategy::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}

void SDNControlledStrategy::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{   
    std::string prefix = nack->GetName().toUri();
    LogDroppedInterest(prefix, inFace);
}

void SDNControlledStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  return ForwardingStrategy::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


