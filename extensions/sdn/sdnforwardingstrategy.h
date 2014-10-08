#ifndef SDNFORWARDINGSTRATEGY_H
#define SDNFORWARDINGSTRATEGY_H

#include <ns3-dev/ns3/ndn-forwarding-strategy.h>
#include <ns3-dev/ns3/ndn-name.h>
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

    void PrintForwardingBase();
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
    void AddNeighbour(Ptr<Face> inFace, Ptr<Data> data);

    //TODO move printing stuff to some logging class
    void PrintControllerMap();
    void PrintNeighbours();


    //TODO move forwarding stuff to forwarding engine class
    bool            FloodInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    bool            ForwardInterestToApp(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    void            ForwardControllerRegistration(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    Ptr<Face>       SelectControllerFace();
    virtual void    AddFibEntry(std::string prefix, int faceId);

private:

    std::vector<Ptr<ndn::Face> > facesList;
    //TODO use a heap structure for the controller faces;
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

    //TODO use more sophisticated forwarding strategies such as proposed by daniel
    std::map<std::string, std::vector<Ptr<Face> > > forwardingBase;
};

}
}
}
#endif // SDNFORWARDINGSTRATEGY_H
