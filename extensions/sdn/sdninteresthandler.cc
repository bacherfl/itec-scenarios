#include "sdninteresthandler.h"
#include <stdio.h>
#include "sdn-ndn.h"

namespace ns3 {
namespace ndn {
namespace sdn {

using namespace std;

SDNInterestHandler::SDNInterestHandler(Ptr<App> owner):
    sequenceNumber(0),
    owner(owner)
{
}

Ptr<Data> SDNInterestHandler::ProcessInterest(Ptr<const ns3::ndn::Interest> interest)
{
    std::string prefix = interest->GetName().getPrefix(1, 0).toUri();
    string interestName = interest->GetName().toUri();

    if ((interestName.find(SdnNdn::CONTROLLER_PREFIX) != string::npos) &&
            (interestName.find(SdnNdn::REGISTER_PREFIX) != string::npos))
    {
        return CreateRouterRegistrationAck(interest);
    }
    else if (interestName.find(SdnNdn::CONTROLLER_PREFIX) != string::npos)
    {
        return CreateControllerDiscoveryResponse(interest);
    }

    return NULL;
}

Ptr<Data> SDNInterestHandler::CreateRouterRegistrationAck(Ptr<const Interest> interest)
{
    Ptr<Data> response = Create<Data();
    response->SetName(interest->GetName);
    response->SetTimestamp(Simulator::Now());

    return response;
}

Ptr<Data> SDNInterestHandler::CreateControllerDiscoveryResponse(Ptr<const Interest> interest)
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

    return response;
}

} //namespace sdn
} //namespace ndn
} //namespace ns3

