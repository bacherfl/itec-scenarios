#ifndef CUSTOMSTRATEGY_H
#define CUSTOMSTRATEGY_H

#include "ns3/log.h"
#include "ns3/ndn-forwarding-strategy.h"
#include "ns3/ndn-l3-protocol.h"

#include <vector>
#include <hash_map>

namespace ns3 {
namespace ndn {
namespace fw {

class CustomStrategy : public ForwardingStrategy
{
public:
    static TypeId
    GetTypeId();

    static std::string
    GetLogName();

    CustomStrategy();

protected:
    virtual bool
    DoPropagateInterest(Ptr<Face> inFace,
                        Ptr<const Interest> interest,
                        Ptr<pit::Entry> pitEntry);

    virtual void
    AddFace(Ptr<Face> face);

    virtual void OnData(Ptr<Face> face, Ptr<Data> data);

    void writeLog();

private:
    std::vector<Ptr<ndn::Face> > facesList;

    typedef std::map<std::string, std::set<Ptr<ndn::Face > > > FacesMap;
    FacesMap knownFaces;

    std::map<int, int> events;

};

} //namespace fw
} //namespace ndn
} //namespace ns3
#endif // CUSTOMSTRATEGY_H
