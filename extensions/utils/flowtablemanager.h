#ifndef FLOWTABLEMANAGER_H
#define FLOWTABLEMANAGER_H

#include "ns3/ndn-face.h"
#include "boost/thread/mutex.hpp"


namespace ns3 {
namespace ndn {
namespace fw {

typedef struct flow_entry_t
{
    int faceId;
    long receivedInterests;
    long satisfiedInterests;
    long unsatisfiedInterests;
    long bytesReceived;
    long status;
    double probability;
} FlowEntry;

typedef struct link_repair_action {
    bool repair;
    double failRate;
} LinkRepairAction;

class FlowTableManager
{
public:
    FlowTableManager();

    void PushRule(const std::string &prefix, int faceId);
    bool TryUpdateFaceProbabilities(const std::string &prefix);
    void AddFlowEntry(const std::string &prefix, FlowEntry *fe);
    double CalculateSuccessRate(FlowEntry *fe);
    void AddFace(Ptr<Face> face);
    LinkRepairAction* InterestUnsatisfied(const std::string &prefix, int faceId);
    LinkRepairAction* InterestSatisfied(const std::string &prefix, int faceId);

    Ptr<Face> GetFaceForPrefix(const std::string &prefix);

private:

    std::vector<Ptr<Face> > faces;
    std::map<std::string, std::vector<FlowEntry* > > flowTable;

    static const double MIN_SAT_RATIO;
    static const int FACE_STATUS_GREEN;
    static const int FACE_STATUS_YELLOW;
    static const int FACE_STATUS_RED;

    boost::mutex mtx_;
};

}
}
}

#endif // FLOWTABLEMANAGER_H
