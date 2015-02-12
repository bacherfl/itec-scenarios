#include "flowtablemanager.h"

namespace ns3 {
namespace ndn {
namespace fw {

using namespace std;

const double FlowTableManager::MIN_SAT_RATIO = 0.9;
const int FlowTableManager::FACE_STATUS_GREEN = 0;
const int FlowTableManager::FACE_STATUS_YELLOW = 1;
const int FlowTableManager::FACE_STATUS_RED = 2;

FlowTableManager::FlowTableManager()
{
}

void FlowTableManager::AddFace(Ptr<Face> face)
{
    faces.push_back(face);
}

void FlowTableManager::PushRule(const string &prefix, int faceId)
{
    mtx_.lock();
    vector<FlowEntry* > flowEntries = flowTable[prefix];

    bool found = false;
    for (vector<FlowEntry *>::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
            found = true;
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
    mtx_.unlock();
    /*
    cout << "Flow Table for " << prefix << ": \n";

    for (int i = 0; i < flowEntries.size(); i++)
    {
        cout << " " << flowEntries.at(i)->faceId << " ";
    }
    */
    cout << "\n";
}

void FlowTableManager::AddFlowEntry(const string &prefix, FlowEntry *fe)
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

bool FlowTableManager::TryUpdateFaceProbabilities(const string &prefix)
{
    vector<FlowEntry *> flowEntries = flowTable[prefix];
    double fractionToShift;
    double shifted;
    bool success = true;
    for (vector<FlowEntry *>::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        double successRate = CalculateSuccessRate(fe);
        if (successRate < MIN_SAT_RATIO)
        {
            fractionToShift = MIN_SAT_RATIO - successRate;
            shifted = 0;
            for (vector<FlowEntry *>::iterator it2 = flowEntries.begin(); it2 != flowEntries.end(); it2++)
            {
                FlowEntry *fe2 = (*it);
                double successRate2 = CalculateSuccessRate(fe2);

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

double FlowTableManager::CalculateSuccessRate(FlowEntry *fe)
{
    double successRate =
            fe->satisfiedInterests + fe->unsatisfiedInterests == 0 ? 1 : (double)fe->satisfiedInterests / (fe->satisfiedInterests + fe->unsatisfiedInterests);

    return successRate;
}

LinkRepairAction* FlowTableManager::InterestUnsatisfied(const string &prefix, int faceId)
{
    vector<FlowEntry* > flowEntries = flowTable[prefix];
    LinkRepairAction *action = new LinkRepairAction;
    action->repair = false;
    for (vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
        {            
            mtx_.lock();
            fe->unsatisfiedInterests++;
            TryUpdateFaceProbabilities(prefix);
            //check if ratio of unsatisfied to satisfied requests exceeds some limit and tell the controller
            double successRate = CalculateSuccessRate(fe);
            mtx_.unlock();
            if ((successRate < MIN_SAT_RATIO) && (fe->status == FACE_STATUS_GREEN))
            {
                fe->status = FACE_STATUS_RED;
                action->repair = true;
                action->failRate = 1 - successRate;
            }
            else {
                action->repair = false;
            }
        }        
    }
    return action;
}

LinkRepairAction* FlowTableManager::InterestSatisfied(const std::string &prefix, int faceId)
{
    LinkRepairAction *action = new LinkRepairAction;
    action->repair = false;
    vector<FlowEntry* > flowEntries = flowTable[prefix];

    for (vector<FlowEntry* >::iterator it = flowEntries.begin(); it != flowEntries.end(); it++)
    {
        FlowEntry *fe = (*it);
        if (fe->faceId == faceId)
        {
            mtx_.lock();
            fe->satisfiedInterests++;
            TryUpdateFaceProbabilities(prefix);
            double successRate = CalculateSuccessRate(fe);
            mtx_.unlock();
            //cout << "satisfied: " << successRate << "\n";
            if ((fe->status == FACE_STATUS_RED) && (successRate >= MIN_SAT_RATIO + 0.05))
            {
                fe->status = FACE_STATUS_GREEN;
                action->repair = true;
                action->failRate = 1 -successRate;
            }
            else {
                action->repair = false;
            }
        }
    }
    return action;
}

Ptr<Face> FlowTableManager::GetFaceForPrefix(const std::string &prefix, int inFaceId)
{
    if (flowTable[prefix].size() > 0)
    {
        //return GetFaceForPrefixBasedOnReliability(prefix, inFaceId);
        return GetFaceForPrefixBasedOnUniformDistribution(prefix, inFaceId);
    }
    return NULL;
}

Ptr<Face> FlowTableManager::GetFaceForPrefixBasedOnReliability(const std::string &prefix, int inFaceId)
{
    double p = (double)rand() / RAND_MAX;
    double tmp = 0.0;
    int faceId;
    for (vector<FlowEntry *>::iterator it = flowTable[prefix].begin(); it != flowTable[prefix].end(); it++)
    {
        FlowEntry *fe = (*it);
        if (p <= tmp + fe->probability)
        {
            faceId = fe->faceId;
            fe->receivedInterests++;
            if (fe->receivedInterests >= 1000)
            {
                mtx_.lock();
                fe->receivedInterests = 0;
                fe->satisfiedInterests = 0;
                fe->unsatisfiedInterests = 0;
                mtx_.unlock();
            }
            break;
        } else {
            tmp += fe->probability;
        }
    }
    for (int i = 0; i < faces.size(); i++)
    {
        Ptr<Face> face = faces.at(i);
        if (face->GetId() == faceId)
        {

            return face;
        }
    }
}

Ptr<Face> FlowTableManager::GetFaceForPrefixBasedOnUniformDistribution(const std::string &prefix, int inFaceId)
{
    bool faceFound = false;
    int cnt = 0;
    int faceId;
    FlowEntry *fe;
    vector<FlowEntry *> candidates;
    for (int i = 0; i < flowTable[prefix].size(); i++)
    {
        candidates.push_back(flowTable[prefix].at(i));
    }
    while ((!faceFound) && (candidates.size() > 0))
    {
        int idx = rand() % candidates.size();
        //flowTable[prefix]
        fe = candidates.at(idx);
        if (fe->faceId != inFaceId)
        {
            faceId = fe->faceId;
            faceFound = true;
        } else {
            candidates.erase(candidates.begin() + idx);
        }
        if (candidates.size() == 0)
        {
            return NULL;
        }
    }


    fe->receivedInterests++;
    if (fe->receivedInterests >= 100)
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

            return face;
        }
    }
}

std::vector<std::string> FlowTableManager::getFlowsOfFace(int faceId)
{
    typedef std::map<std::string, std::vector<FlowEntry* > > FlowTable;
    vector<string> flows;
    for (FlowTable::iterator it = flowTable.begin(); it != flowTable.end(); it++) {
        std::vector<FlowEntry *> flowEntries = it->second;
        for (std::vector<FlowEntry *>::iterator it2 = flowEntries.begin(); it2 != flowEntries.end(); it2++) {
            FlowEntry *fe = (*it2);
            if (fe->faceId == faceId) {
                flows.push_back(it->first);
            }
        }
    }

    return flows;
}

}
}
}
