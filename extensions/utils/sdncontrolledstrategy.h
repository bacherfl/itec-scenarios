#ifndef SDNCONTROLLEDSTRATEGY_H
#define SDNCONTROLLEDSTRATEGY_H

#include "ns3-dev/ns3/log.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"

#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-fib-limits.h"
#include "../../../ns-3/src/ndnSIM/model/fw/nacks.h"

#include "sdncontroller.h"

#include "boost/foreach.hpp"
#include "boost/thread/mutex.hpp"

#include "qosqueue.h"

#include <stdio.h>

#include "forwardingengine.h"
#include "flowtablemanager.h"

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SDNControlledStrategy
class SDNControlledStrategy: public Nacks
{

public:
    static TypeId GetTypeId ();

    static std::string GetLogName ();

    SDNControlledStrategy ();

    virtual void AddFace(Ptr< Face> face);
    virtual void RemoveFace(Ptr< Face > face);
    virtual void OnInterest(Ptr< Face > inFace, Ptr< Interest > interest);
    virtual bool DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    virtual void WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
    virtual void WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
    virtual void DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);
    virtual void DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry);
    //virtual bool TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);
    virtual void DidExhaustForwardingOptions(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    //virtual void OnData(Ptr<Face> face, Ptr<Data> data);

    void init();
    void PushRule(const std::string &prefix, int faceId, int cost);
    void AssignBandwidth(const std::string &prefix, int faceId, uint64_t bitrate);
    std::vector<std::string> getFlowsOfFace(int faceId);
    //bool TryUpdateFaceProbabilities(std::vector<FlowEntry* > flowEntries);
    //void AddFlowEntry(const std::string &prefix, FlowEntry *fe);

    Ptr<Face> GetFaceFromSDNController(Ptr<const Interest> interest, int inFaceId);
    Ptr<Face> SelectFaceFromLocalFib(Ptr<const Interest> interest, std::vector<int> exclude);

    Ptr<Interest> prepareNack(Ptr<const Interest> interest);

    void LogDroppedInterest(std::string prefix, int faceId);
protected:

    int m_maxLevel;
    static LogComponent g_log;

    std::vector<Ptr<ndn::Face> > faces;
    Ptr<utils::ForwardingEngine> fwEngine;

    std::map<std::string, std::vector<int> > localFib; //TODO: replace with flowTable

    FlowTableManager flowTableManager;

    std::map<std::string, std::vector<FlowEntry* > > flowTable;

    std::map<int, std::map<std::string, ns3::ndn::utils::QoSQueue*> > qosQueues;
    std::map<int, ns3::ndn::utils::QoSQueue*> aggregateQosQueues;
    std::map<int, bool> qosQueueInitialized;
    std::map<std::string, int> pitTable;


    unsigned int prefixComponentNum;
    unsigned int useTockenBucket;
    bool initialized;
    bool useAggregateQueuesPerFace;

    static const double MIN_SAT_RATIO;
    static const int FACE_STATUS_GREEN;
    static const int FACE_STATUS_YELLOW;
    static const int FACE_STATUS_RED;

    boost::mutex mtx_;


    bool HasQueue(int faceId, std::string prefix);
    bool TryConsumeQueueToken(int faceId, std::string prefix);

};

}
}
}

#endif // SDNCONTROLLEDSTRATEGY_H
