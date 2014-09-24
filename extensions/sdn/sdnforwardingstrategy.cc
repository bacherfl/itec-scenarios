#include "sdnforwardingstrategy.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-fib-entry.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/core-module.h"
#include <stdio.h>
#include<jsoncpp/json/json.h>

namespace ns3 {
namespace ndn {
namespace fw {

using namespace std;

NS_OBJECT_ENSURE_REGISTERED (SDNForwardingStrategy);

SDNForwardingStrategy::SDNForwardingStrategy()
{
    init();
}

bool SDNForwardingStrategy::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    std::string prefix = interest->GetName().getPrefix(1, 0).toUri();
    Ptr<Node> node = this->GetObject<Node>();

    if (prefix.compare("/controller") == 0)
    {
        return DiscoverController(inFace, interest, pitEntry);
    }
    else if (prefix.compare("/neighbours") == 0)
    {
        //if the interest comes from the app face, forward it to all neighbours
        if (ndn::Face::APPLICATION & inFace->GetFlags() != 0)
        {
            std::cout << node->GetId() << " received neighbour discovery interest from app " << interest->GetName().toUri() << "\n";
            DiscoverNeighbours(inFace, interest, pitEntry);
        }
        //if the interest comes from an external face, send it to the app layer
        else {
            std::cout << node->GetId() << " received neighbour discovery interest from external face " << interest->GetName().toUri() << "\n";

            ForwardNeighbourDiscoveryInterestToApp(inFace, interest, pitEntry);
        }
    }
}

void SDNForwardingStrategy::AddFace(Ptr<Face> face)
{
    facesList.push_back(face);
    ForwardingStrategy::AddFace(face);
}

void SDNForwardingStrategy::OnData(Ptr<Face> face, Ptr<Data> data)
{
    //TODO move response handling to separate handler class
    //check for special controller prefix
    std::string prefix = data->GetName().getPrefix(1, 0).toUri();

    if (prefix.compare("/controller") == 0)
    {
        //received controller response
        uint8_t *buf = (uint8_t*)(malloc (sizeof(uint8_t) * data->GetPayload()->GetSize()));
        data->GetPayload()->CopyData(buf, data->GetPayload()->GetSize());
        std::string jsonString(reinterpret_cast<char const*>(buf));

        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(jsonString, root))
        {
            std::cout << "Error while parsing controller response";
            return;
        }

        const std::string controllerId = root.get("id", "unknown").asString();
        int seqNr = root.get("seq", 0).asInt();

        Ptr<Node> node = this->GetObject<Node>();

        std::cout << node->GetId() << " Found controller " << controllerId << " via face " << face->GetId() << ", seq=" << seqNr << "\n";

        AddControllerLocation(controllerId, face);
    }
    else if (prefix.compare("/neighbours") == 0)
    {
        //don't add the app face as a neighbour
        AddNeighbour(face, data);
    }

    ForwardingStrategy::OnData(face, data);
}

void SDNForwardingStrategy::WillEraseTimedOutPendingInterest(Ptr<pit::Entry> pitEntry)
{
    std::string prefix = pitEntry->GetInterest()->GetName().getPrefix(1,0).toUri();
    //pitEntry->GetOutgoing()
    if (prefix.compare("/controller") == 0)
    {
        //std::cout << "Controller not reachable \n";
        //RemoveControllerEntry();
    }
}

void SDNForwardingStrategy::AddControllerLocation(std::string controllerId, Ptr<Face> face)
{
    //get RTT to controller via this face
    int64_t rtt = Simulator::Now().GetMilliSeconds() - controllerDiscoveryStartTime;
    if (controllerMap.count(controllerId) == 0)
    {
        ControllerFaceEntry controllerFaces;
        controllerMap[controllerId] = controllerFaces;
    }

    controllerMap[controllerId][face] = rtt;
    //PrintControllerMap();
}

void SDNForwardingStrategy::AddNeighbour(Ptr<Face> inFace, Ptr<Data> data)
{
    if (Face::APPLICATION & inFace->GetFlags() == 0)
    {
        uint8_t *buf = (uint8_t*)(malloc (sizeof(uint8_t) * data->GetPayload()->GetSize()));
        data->GetPayload()->CopyData(buf, data->GetPayload()->GetSize());
        std::string jsonString(reinterpret_cast<char const*>(buf));

        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(jsonString, root))
        {
            std::cout << "Error while parsing controller response";
            return;
        }

        int64_t rtt = Simulator::Now().GetMilliSeconds() - neighbourDiscoveryStartTime;
        int neighbourId = root.get("id", 0).asInt();

        std::cout << "Adding neighbour " << neighbourId << "\n";

        //TODO add RTT
        NeighbourEntry entry;
        entry.face = inFace;
        entry.rtt = rtt;
        neighbours[neighbourId] = entry;
        PrintNeighbours();
    }

}

TypeId SDNForwardingStrategy::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ndn::fw::SDNForwardingStrategy")
            .SetGroupName("Ndn")
            .SetParent<ForwardingStrategy>()
            .AddConstructor<SDNForwardingStrategy>();

    return tid;
}

std::string SDNForwardingStrategy::GetLogName()
{
    return "ndn.fw.SDNForwardingStrategy";
}

void SDNForwardingStrategy::init()
{

}

/**
 Controller Discovery: Flood interest for controller to all outgoing faces
**/
bool SDNForwardingStrategy::DiscoverController(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{    
    controllerDiscoveryStartTime = Simulator::Now().GetMilliSeconds();
    return FloodInterest(inFace, interest, pitEntry);
}

bool SDNForwardingStrategy::DiscoverNeighbours(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    neighbourDiscoveryStartTime = Simulator::Now().GetMilliSeconds();
    return FloodInterest(inFace, interest, pitEntry);
}

bool SDNForwardingStrategy::ForwardNeighbourDiscoveryInterestToApp(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    for (std::vector<Ptr<ndn::Face> >::iterator it = facesList.begin(); it != facesList.end(); it++)
    {
        Ptr<Face> face = *it;

        if (Face::APPLICATION & face->GetFlags() != 0)
        {
            if (TrySendOutInterest(inFace, face, interest, pitEntry)){
                std::cout << "sending to app face \n";
                return true;
            }

        }
    }
    return false;
}

bool SDNForwardingStrategy::FloodInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    int propagatedCount = 0;
    std::vector<Ptr<Face> > sentFaces;
    for (std::vector<Ptr<Face> >::iterator it = facesList.begin(); it != facesList.end(); it++)
    {
        Ptr<Face> face = *it;
        // TODO: write custom producer that checks prefixes of incoming interests before sending data
        if ((inFace->GetId() != face->GetId()))
        {
            if ((TrySendOutInterest(inFace, face, interest, pitEntry)))
            {
                sentFaces.push_back(face);
                propagatedCount++;
            }
        }
    }
    Ptr<Node> node = this->GetObject<Node>();
    for (std::vector<Ptr<Face> >::iterator it = sentFaces.begin(); it != sentFaces.end(); it++)
    {
        Ptr<Face> face = *it;

        cout << node->GetId() << " sent interest " << interest->GetName().toUri() << " to face " << face->GetId() << "\n";
    }
    return propagatedCount > 0;
}

void SDNForwardingStrategy::PrintControllerMap()
{
    Ptr<Node> node = this->GetObject<Node>();
    std::cout << "Controller map of node " << node->GetId() << ":\n";
    std::cout << "================================================\n";
    for (ControllerMap::iterator it = controllerMap.begin(); it != controllerMap.end(); it++)
    {
        std::cout << "Controller " << it->first << "\n";
        for (ControllerFaceEntry::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            Ptr<Face> face = it2->first;
            std::cout << "   " << face->GetId() << ": RTT = " << it2->second << " \n";
        }
    }
    std::cout << "================================================\n";
}

void SDNForwardingStrategy::PrintNeighbours()
{
    Ptr<Node> node = this->GetObject<Node>();

    std::cout << "Neighbours of node " << node->GetId() << ":\n";
    std::cout << "================================================\n";
    for (std::map<int, NeighbourEntry>::iterator it = neighbours.begin(); it != neighbours.end(); it++)
    {
        std::cout << it->first << ": Face = " << it->second.face->GetId() << ", RTT = " << it->second.rtt << "\n";
    }

    std::cout << "================================================\n";

}

}
}
}
