#ifndef SDNFORWARDINGSTRATEGY_H
#define SDNFORWARDINGSTRATEGY_H

#include <ns3-dev/ns3/ndn-forwarding-strategy.h>
#include <vector>
#include <map>

namespace ns3 {
namespace ndn {
namespace fw {

class SDNForwardingStrategy : public ForwardingStrategy
{
public:
    static TypeId
    GetTypeId();

    static std::string
    GetLogName();

    SDNForwardingStrategy();

protected:
    virtual bool
    DoPropagateInterest(Ptr<Face> inFace,
                        Ptr<const Interest> interest,
                        Ptr<pit::Entry> pitEntry);

    virtual void
    AddFace(Ptr<Face> face);

    virtual void OnData(Ptr<Face> face, Ptr<Data> data);
    virtual void WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);

    void init();
    bool DiscoverController(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    bool DiscoverNeighbours(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    void AddControllerLocation(std::string controllerId, Ptr<Face> face);
    void PrintControllerMap();
    bool FloodInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    bool ForwardNeighbourDiscoveryInterestToApp(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    void AddNeighbour(Ptr<Face> inFace, Ptr<Data> data);
    void PrintNeighbours();

private:

    std::vector<Ptr<ndn::Face> > facesList;
    typedef std::map<Ptr<Face>, int64_t > ControllerFaceEntry; //controller face -> RTT
    typedef std::map<std::string, ControllerFaceEntry > ControllerMap;
    ControllerMap controllerMap;

    typedef struct NeighbourEntry {
        int64_t rtt;
        Ptr<Face> face;
    } NeighbourEntry;

    typedef std::map<int, NeighbourEntry> NeighbourMap;
    NeighbourMap neighbours;

    int64_t controllerDiscoveryStartTime;
    int64_t neighbourDiscoveryStartTime ;
};

}
}
}
#endif // SDNFORWARDINGSTRATEGY_H
