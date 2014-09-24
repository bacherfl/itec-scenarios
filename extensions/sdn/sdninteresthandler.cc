#include "sdninteresthandler.h"
#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace sdn {

SDNInterestHandler::SDNInterestHandler(Ptr<App> owner):
    sequenceNumber(0),
    owner(owner)
{
}

Ptr<Data> SDNInterestHandler::ProcessInterest(Ptr<const ns3::ndn::Interest> interest)
{
    std::string prefix = interest->GetName().getPrefix(1, 0).toUri();

    //Case 1: Controller discovery insterest
    if (prefix.compare("/controller") == 0)
    {
        return SendControllerDiscoveryResponse(interest);
    }
    return NULL;
}

Ptr<Data> SDNInterestHandler::SendControllerDiscoveryResponse(Ptr<const Interest> interest)
{
    Json::Value responseContent = Json::Value(Json::objectValue);
    std::stringstream controllerIdStream;
    controllerIdStream << "controller" << owner->GetNode()->GetId();
    responseContent["id"] = controllerIdStream.str();
    responseContent["seq"] = sequenceNumber++;

    Json::FastWriter writer;

    Ptr<Data> response = Create<Data> (Create<Packet> (writer.write(responseContent)));
    response->SetName(interest->GetName());
    response->SetTimestamp(Simulator::Now());
    //response->GetPayload()->CopyData()
    uint8_t *buffer = (uint8_t*)(malloc (sizeof(uint8_t) * response->GetPayload()->GetSize()));

    response->GetPayload()->CopyData(buffer, response->GetPayload()->GetSize());

    std::string s (reinterpret_cast<char const*>(buffer));

    std::cout << "Packet payload = " << s << "\n";

    return response;
}

} //namespace sdn
} //namespace ndn
} //namespace ns3

