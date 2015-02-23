#include "sdncontrolledstrategy.h"

namespace ns3 {
namespace ndn {
namespace fw {

using namespace std;

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


    faces.push_back (face);//TODO: obsolete
    Nacks::AddFace(face);
}

void SDNControlledStrategy::AssignBandwidth(const string &prefix, int faceId, uint64_t bitrate)
{
    qosQueues[faceId][prefix] = new ns3::ndn::utils::QoSQueue(bitrate);
}

void SDNControlledStrategy::PushRule(const string &prefix, int faceId, int cost)
{
    //cout << "new rule: " << prefix << " -> face " << faceId << "\n";

    flowTableManager.PushRule(prefix, faceId, cost);
    /*

    vector<FlowEntry* > flowEntries = flowTable[prefix];

    bool found = false;
    for (vector<FlowEntry *>::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
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
        fe->probability = 0.0;
        AddFlowEntry(prefix, fe);
        //flowTable[prefix].push_back(fe);
    }
    */
}

/*
void SDNControlledStrategy::AddFlowEntry(const string &prefix, FlowEntry *fe)
{
    vector <FlowEntry *> flowEntries = flowTable[prefix];
    flowEntries.push_back(fe);
    double shift = 1.0 / flowEntries.size();
    for (vector<FlowEntry *>::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *tmp = (*it);
        if (tmp->faceId != fe->faceId)
        {
            tmp->probability = max(tmp->probability - shift, 0.0);
        }
        else {
            tmp->probability += shift;
        }
    }
    flowTable[prefix] = flowEntries;
}
*/

/*
bool SDNControlledStrategy::TryUpdateFaceProbabilities(vector<FlowEntry *> flowEntries)
{
    double fractionToShift;
    double shifted;
    bool success = true;
    for (vector<FlowEntry *>::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        double successRate =
                fe->satisfiedInterests + fe->unsatisfiedInterests == 0 ? 1 : (double)fe->satisfiedInterests / (fe->satisfiedInterests + fe->unsatisfiedInterests);
        if (successRate < MIN_SAT_RATIO)
        {
            fractionToShift = MIN_SAT_RATIO - successRate;
            shifted = 0;
            for (vector<FlowEntry *>::iterator it2 = flowEntries.begin(); it2 != flowEntries.end(); it2++)
            {
                FlowEntry *fe2 = (*it);
                double successRate2 = fe2->satisfiedInterests + fe2->unsatisfiedInterests == 0 ? 1 : (double)fe2->satisfiedInterests / (fe2->satisfiedInterests + fe2->unsatisfiedInterests);

                if (successRate2 > MIN_SAT_RATIO)
                {
                    double shift = min(fractionToShift, successRate2 - MIN_SAT_RATIO);
                    shift = min(shift, 1 - fe2->probability);
                    fe->probability -= shift;
                    fe2->probability += shift;
                    fractionToShift -= shift;
                    shifted += shift;
                }
            }
            if (fractionToShift - shifted > 0)
                success = false;
        }
    }
    return success;
}
*/
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

    return SelectFaceFromLocalFib(interest, inFaceId);
}

Ptr<Face> SDNControlledStrategy::SelectFaceFromLocalFib(Ptr<const Interest> interest, int inFaceId)
{
    string prefix = interest->GetName().getPrefix(interest->GetName().size() - 1).toUri();
    return flowTableManager.GetFaceForPrefix(prefix, inFaceId);
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

bool SDNControlledStrategy::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    Ptr<Face> outFace = SelectFaceFromLocalFib(interest, inFace->GetId());

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
        /*
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
        */
    }

    return propagatedCount > 0;
}

void SDNControlledStrategy::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
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
    Nacks::WillSatisfyPendingInterest(inFace,pitEntry);
}

void SDNControlledStrategy::OnData(Ptr<Face> face, Ptr<Data> data)
{    
    Nacks::OnData(face, data);
}

void SDNControlledStrategy::DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
    Nacks::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}

void SDNControlledStrategy::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{   
    cout << "Received NACK \n";
    Name name = pitEntry->GetInterest()->GetName();
    string prefix = name.getPrefix(name.size() - 1).toUri();
    LogDroppedInterest(prefix, inFace->GetId());
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


