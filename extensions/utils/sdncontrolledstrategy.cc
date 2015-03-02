#include "sdncontrolledstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

using namespace std;

NS_OBJECT_ENSURE_REGISTERED (SDNControlledStrategy);

LogComponent SDNControlledStrategy::g_log = LogComponent (SDNControlledStrategy::GetLogName ().c_str ());
const double SDNControlledStrategy::MIN_SAT_RATIO = 0.9;
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
    useAggregateQueuesPerFace = false;
}

TypeId SDNControlledStrategy::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ndn::fw::SDNControlledStrategy")
        .SetGroupName ("Ndn")
        .SetParent <Nacks> ()
        .AddConstructor <SDNControlledStrategy> ();
    return tid;
}

string SDNControlledStrategy::GetLogName ()
{
  return Nacks::GetLogName () + ".SDNControlledStrategy";
}

void SDNControlledStrategy::AddFace (Ptr<Face> face)
{
    if (!initialized)
        this->init();
    flowTableManager.AddFace(face);


    faces.push_back (face);
    qosQueueInitialized[face->GetId()] = false;
    Nacks::AddFace(face);
}

void SDNControlledStrategy::AssignBandwidth(const string &prefix, int faceId, uint64_t bitrate)
{
    //qosQueues[faceId][prefix] = new ns3::ndn::utils::QoSQueue(bitrate);    
    mtx_.lock();

    if (!aggregateQosQueues[faceId])
        aggregateQosQueues[faceId] = new ns3::ndn::utils::QoSQueue(bitrate);

    typedef std::map<int, std::map<std::string, ns3::ndn::utils::QoSQueue*> > QoSQueues;
    for (QoSQueues::iterator it = qosQueues.begin(); it != qosQueues.end(); it++) {
        vector<string> flows = flowTableManager.getFlowsOfFace(it->first);
        int nrFlows = flows.size();
        nrFlows = nrFlows > 0 ? nrFlows : 1;
        int bitRateLimit = bitrate / nrFlows;

        for (vector<string>::iterator it2 = flows.begin(); it2 != flows.end(); it2++) {
            qosQueues[it->first][*it2] = new ns3::ndn::utils::QoSQueue(bitRateLimit);
        }

    }
    mtx_.unlock();
}

void SDNControlledStrategy::PushRule(const string &prefix, int faceId, int cost)
{
    //cout << "new rule: " << prefix << " -> face " << faceId << "\n";
    flowTableManager.PushRule(prefix, faceId, cost);
}

void SDNControlledStrategy::RemoveFace (Ptr<Face> face)
{
    for (vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
         it !=  faces.end (); ++it)
    {
        if (face->GetId ()== (*it)->GetId())
        {
            faces.erase (it);
            break;
        }
    }
    Nacks::RemoveFace(face);
}

void SDNControlledStrategy::OnInterest (Ptr< Face > inFace, Ptr< Interest > interest)
{
    Nacks::OnInterest(inFace,interest);
}

Ptr<Interest> SDNControlledStrategy::prepareNack(Ptr<const Interest> interest)
{
    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (ndn::Interest::NACK_CONGESTION); // set this since ndn changes it anyway to this.
    return nack;
}

Ptr<Face> SDNControlledStrategy::GetFaceFromSDNController(Ptr<const Interest> interest, int inFaceId)
{
    string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
    Ptr<Node> node = GetObject<Node>();
    //let the controller calculate the route and push the rules to all nodes on the path to the target
    SDNController::CalculateRoutesForPrefix(node->GetId(), prefix);

    vector<int> exclude;
    exclude.push_back(inFaceId);
    return SelectFaceFromLocalFib(interest, exclude);
}

Ptr<Face> SDNControlledStrategy::SelectFaceFromLocalFib(Ptr<const Interest> interest, vector<int> exclude)
{
    string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
    return flowTableManager.GetFaceForPrefix(prefix, exclude);
}

void SDNControlledStrategy::LogDroppedInterest(string prefix, int faceId)
{
    LinkRepairAction *action = flowTableManager.InterestUnsatisfied(prefix, faceId);

    if (action->repair)
    {
        Ptr<Node> node = this->GetObject<Node>();
        SDNController::LinkFailure(node->GetId(), faceId, prefix, action->failRate);
    }
}

bool SDNControlledStrategy::HasQueue(int faceId, string prefix)
{
    if (useAggregateQueuesPerFace) {
        return qosQueueInitialized[faceId];
    } else {
        return qosQueues[faceId].size() > 0 && qosQueues[faceId][prefix] != NULL;
    }
}

bool SDNControlledStrategy::TryConsumeQueueToken(int faceId, string prefix)
{
    if (useAggregateQueuesPerFace) {
        return aggregateQosQueues[faceId]->TryForwardInterest();
    } else {
        return qosQueues[faceId][prefix]->TryForwardInterest();
    }
}

bool SDNControlledStrategy::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    vector<int> exclude;
    exclude.push_back(inFace->GetId());
    Ptr<Face> outFace = SelectFaceFromLocalFib(interest, exclude);

    int propagatedCount = 0;

    if (outFace == NULL)
    {
        outFace = GetFaceFromSDNController(interest, inFace->GetId());
    }

    if (outFace != NULL)
    {        
        string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
        if (outFace->GetId() == faces.size() - 1)
        {
            if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                propagatedCount++;
        }

        else if (HasQueue(outFace->GetId(), prefix))
        {
            pitTable[interest->GetName().toUri()] = outFace->GetId();

            if (TryConsumeQueueToken(outFace->GetId(), prefix))
            {
                if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                    propagatedCount++;
            }
            else {
                bool tryForwarding = true;
                while (tryForwarding) {
                    exclude.push_back(outFace->GetId());
                    outFace = SelectFaceFromLocalFib(interest, exclude);
                    if (outFace == NULL) {
                        tryForwarding = false;
                        break;
                    } else {
                        if (outFace->GetId() == faces.size() - 1)
                        {
                            if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                                propagatedCount++;
                        }

                        else if (HasQueue(outFace->GetId(), prefix))
                        {
                            pitTable[interest->GetName().toUri()] = outFace->GetId();

                            if (TryConsumeQueueToken(outFace->GetId(), prefix))
                            {
                                if (TrySendOutInterest (inFace, outFace, interest, pitEntry)) {
                                    propagatedCount++;
                                    tryForwarding = false;
                                }
                            }
                        }
                    }
                }
                //if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
                  //  propagatedCount++;
                //LogDroppedInterest(prefix, outFace->GetId());
            }
        }

        else if (TrySendOutInterest (inFace, outFace, interest, pitEntry))
            propagatedCount++;
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
    //cout << "Interest timeout " <<  pitEntry->GetInterest()->GetName().toUri() << endl;
    /*
    int faceId = pitTable[pitEntry->GetInterest()->GetName().toUri()];
    Name name = pitEntry->GetInterest()->GetName();
    string prefix = name.getPrefix(name.size() - 1).toUri();

    vector<FlowEntry* > flowEntries = flowTable[prefix];

    for (vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
        {
            LogDroppedInterest(prefix, faceId);
        }
    }
    */
    Nacks::WillEraseTimedOutPendingInterest(pitEntry);
}

void SDNControlledStrategy::WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
    Nacks::WillSatisfyPendingInterest(inFace,pitEntry);
    if(inFace != 0) // ==0 means data comes from cache
    {
        Name name = pitEntry->GetInterest()->GetName();
        string prefix = name.getPrefix(name.size() - 1).toUri();
        LinkRepairAction *action = flowTableManager.InterestSatisfied(prefix, inFace->GetId());
        if (action->repair)
        {
            Ptr<Node> node = this->GetObject<Node>();
            SDNController::LinkRecovered(node->GetId(), inFace->GetId(), prefix, action->failRate);
        }
    }    
}

/*
void SDNControlledStrategy::OnData(Ptr<Face> face, Ptr<Data> data)
{
    //Ptr<Node> node = this->GetObject<Node>();
    //if (node->GetId() == 28)
    //    cout << "Received data: " << data->GetName().toUri() << endl;
    Nacks::OnData(face, data);
}
*/
void SDNControlledStrategy::DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
    Nacks::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}

void SDNControlledStrategy::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{   
    //cout << "Received NACK \n";
    Name name = pitEntry->GetInterest()->GetName();
    string prefix = name.getPrefix(name.size() - 1).toUri();
    LogDroppedInterest(prefix, inFace->GetId());
    Nacks::DidReceiveValidNack(inFace, nackCode, nack, pitEntry);
}

void SDNControlledStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    Nacks::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

std::vector<std::string> SDNControlledStrategy::getFlowsOfFace(int faceId)
{
    return flowTableManager.getFlowsOfFace(faceId);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


