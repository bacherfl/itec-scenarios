#include "sdnforwardingstrategy.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-fib-entry.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/core-module.h"
#include <stdio.h>
#include <jsoncpp/json/json.h>
#include "sdn-ndn.h"

namespace ns3 {
namespace ndn {
namespace fw {

using namespace std;
using namespace ns3::ndn::sdn;

NS_OBJECT_ENSURE_REGISTERED (SDNForwardingStrategy);

SDNForwardingStrategy::SDNForwardingStrategy()
{
    init();
}

bool SDNForwardingStrategy::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    std::string prefix = interest->GetName().toUri();
    Ptr<Node> node = this->GetObject<Node>();

    cout << node->GetId() << " received interest " << prefix << "\n";
    if ((prefix.find(SdnNdn::CONTROLLER_PREFIX) != std::string::npos) &&
            (prefix.find(SdnNdn::REGISTER_PREFIX) != std::string::npos))
    {
        cout << "received controller registration interest " << prefix  << "\n";
        //received controller registration interest
        //get the id if the sdn router and add it to the FIB
        string routerId = interest->GetName().getPostfix(1, 0).toUri();

        if (forwardingBase.count(routerId) == 0)
        {
            vector<Ptr<Face> > faces;
            forwardingBase[routerId] = faces;
        }
        forwardingBase[routerId].push_back(inFace);
        PrintForwardingBase();
        ForwardControllerRegistration(inFace, interest, pitEntry);        
    }
    else if (prefix.find(SdnNdn::CONTROLLER_PREFIX) != string::npos)
    {
        cout << "received controller discovery response " << prefix << "\n";
        return DiscoverController(inFace, interest, pitEntry);
    }
    else if (prefix.find(SdnNdn::CONTROLLER_NEIGHBOUR_REQUEST) != string::npos)
    {
        cout << "received controller neighbour request " << prefix << "\n";
        string routerId = interest->GetName().getPostfix(1,0).toUri();

        stringstream strNodeId;
        strNodeId <<  "" << node->GetId();

        cout << "received neighbour interest, my id=" << strNodeId.str() << ", interest router id=" << routerId.substr(1) << "\n";
        if (routerId.substr(1).compare(strNodeId.str()) == 0)
        {
            cout << "Forwarding neighbour request from controller to app layer";
            return ForwardInterestToApp(inFace, interest, pitEntry);
        }
        else {
            cout << "Forwarding neighbour request from controller to face " << forwardingBase[routerId][0]->GetId() << "\n";
            if (TrySendOutInterest(inFace, forwardingBase[routerId][0], interest, pitEntry))
            {
                cout << "neighbour interest forwarded \n";
                return true;
            }
            cout << "error while forwarding neighbour interest \n";
            return false;
        }
    }
    else if (prefix.find(SdnNdn::NEIGHBOURS_PREFIX) != string::npos)
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

            ForwardInterestToApp(inFace, interest, pitEntry);
        }
    }
}

void SDNForwardingStrategy::PrintForwardingBase()
{
    Ptr<Node> node = this->GetObject<Node>();
    cout << "Forwarding base of node " << node->GetId() << "\n";
    for (map<string, vector<Ptr<Face> > >::iterator it = forwardingBase.begin(); it != forwardingBase.end(); it++)
    {
        cout << "Router " << it->first << ": ";
        for (vector<Ptr<Face> >::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            Ptr<Face> face = *it2;
            cout << face->GetId() << ", ";
        }
        cout << "\n";
    }
}

void SDNForwardingStrategy::ForwardControllerRegistration(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    //select the face with the least RTT for the controller
    Ptr<Face> ctrlFace = SelectControllerFace();
    TrySendOutInterest(inFace, ctrlFace, interest, pitEntry);
}

Ptr<Face> SDNForwardingStrategy::SelectControllerFace()
{
    Ptr<Node> node = this->GetObject<Node>();
    cout << "selecting controller face on node " << node->GetId() << "\n";
    PrintControllerMap();
    ControllerFaceEntry controllerFaces = controllerMap.begin()->second;

    int64_t max = std::numeric_limits<int64_t>::max();
    Ptr<Face> ctrlFace;
    for (ControllerFaceEntry::iterator it = controllerFaces.begin(); it != controllerFaces.end(); it++)
    {
        if (it->second < max)
        {
            ctrlFace = it->first;
            max = it->second;
        }
    }
    return ctrlFace;
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
    cout << "received data " << data->GetName().toUri() << "\n";
    if ((data->GetName().toUri().find(SdnNdn::CONTROLLER_PREFIX) != std::string::npos) &&
            (data->GetName().toUri().find(SdnNdn::REGISTER_PREFIX) != std::string::npos))
    {
        //received controller registration ack
        cout << "received controller registration ack \n";
    }
    else if (prefix.find(SdnNdn::CONTROLLER_PREFIX) != std::string::npos)
    {
        //received controller response
        Json::Value root = SdnNdn::GetContentObjectJson(data);

        if (!root.isNull())
        {
            const std::string controllerId = root.get("id", "unknown").asString();
            int seqNr = root.get("seq", 0).asInt();

            Ptr<Node> node = this->GetObject<Node>();

            std::cout << node->GetId() << " Found controller " << controllerId << " via face " << face->GetId() << ", seq=" << seqNr << "\n";

            AddControllerLocation(controllerId, face);
        }
    }
    else if (prefix.compare(SdnNdn::NEIGHBOURS_PREFIX) == 0)
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
    if (prefix.compare(SdnNdn::CONTROLLER_PREFIX) == 0)
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
    PrintControllerMap();
}

void SDNForwardingStrategy::AddNeighbour(Ptr<Face> inFace, Ptr<Data> data)
{
    if (Face::APPLICATION & inFace->GetFlags() == 0)
    {
        Json::Value root = SdnNdn::GetContentObjectJson(data);

        if (!root.isNull())
        {
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

bool SDNForwardingStrategy::ForwardInterestToApp(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
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

void SDNForwardingStrategy::AddFibEntry(string prefix, int faceId)
{
    //TODO implement method
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
