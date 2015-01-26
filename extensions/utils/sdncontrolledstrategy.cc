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

    std::vector<FlowEntry* > flowEntries = flowTable[prefix];

    bool found = false;
    for (std::vector<FlowEntry *>::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
            found = true;
        break;
    }
    if (!found)
    {
        FlowEntry *fe = new FlowEntry;
        fe->bytesReceived = 0;
        fe->faceId = faceId;
        fe->receivedInterests = 0;
        fe->satisfiedInterests = 0;
        fe->unsatisfiedInterests = 0;
        fe->status = FACE_STATUS_GREEN;
        flowTable[prefix].push_back(fe);
    }
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
    if (flowTable[prefix].size() > 0)
    {
        int idx = rand() % flowTable[prefix].size();
        //flowTable[prefix]
        FlowEntry *fe = flowTable[prefix].at(idx);
        int faceId = fe->faceId;
        if (faceId != -1)
        {
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
                if (face->GetId() == faceId)
                {
                    Ptr<Node> node = this->GetObject<Node>();
                    return face;
                }
            }
            fe->receivedInterests++;
        }
    }
    return NULL;
}

void SDNControlledStrategy::LogDroppedInterest(std::string prefix, int faceId)
{
    std::vector<FlowEntry* > flowEntries = flowTable[prefix];

    for (std::vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
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
        if (outFace->GetId() == faces.size() - 1)
        {
            if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                propagatedCount++;
        }
        else if (qosQueues[outFace->GetId()].size() > 0 && qosQueues[outFace->GetId()][prefix] != NULL)
        {
            pitTable[interest->GetName().toUri()] = outFace->GetId();
            //===============================================================
            /*
            if (qosQueues[outFace->GetId()][prefix]->TryForwardInterest())
            {
                if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                    propagatedCount++;
            }
            else {
                LogDroppedInterest(prefix, outFace->GetId());
            }
            */
            //===============================================================
            //===================EXPERIMENTAL================================
            if (qosQueues[outFace->GetId()][prefix]->TryForwardInterest())
            {
                if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                    propagatedCount++;
            }
            else {
                if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                    propagatedCount++;
                LogDroppedInterest(prefix, outFace->GetId());
            }
            //===============================================================
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
            if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
                propagatedCount ++;

            faceIterator++;

        }
    }

    return propagatedCount > 0;
}

void SDNControlledStrategy::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
    int faceId = pitTable[pitEntry->GetInterest()->GetName().toUri()];
    Name name = pitEntry->GetInterest()->GetName();
    std::string prefix = name.getPrefix(name.size() - 1).toUri();

    std::vector<FlowEntry* > flowEntries = flowTable[prefix];

    for (std::vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
        {
            LogDroppedInterest(prefix, faceId);
        }
    }

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
                //std::cout << "satisfied: " << successRate << "\n";
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
    LogDroppedInterest(prefix, inFace->GetId());
}

void SDNControlledStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    ForwardingStrategy::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


