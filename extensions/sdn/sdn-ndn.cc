#include "sdn-ndn.h"

namespace ns3 {
namespace ndn {
namespace sdn {

const std::string SdnNdn::CONTROLLER_PREFIX             = "/controller";
const std::string SdnNdn::NEIGHBOURS_PREFIX             = "/neighbours";
const std::string SdnNdn::REGISTER_PREFIX               = "/register";
const std::string SdnNdn::CONTROLLER_REGISTER           = "/controller/register";
const std::string SdnNdn::CONTROLLER_NEIGHBOUR_REQUEST  = "/router/neighbours/all";

std::string SdnNdn::GetContentObjectString(Ptr<const Data> contentObject)
{
    uint8_t *buf = (uint8_t*) malloc(sizeof(uint8_t) * contentObject->GetPayload()->GetSize());
    contentObject->GetPayload()->CopyData(buf, contentObject->GetPayload()->GetSize());

    std::string responseStr = reinterpret_cast<char const*>(buf);
    return responseStr;
}

Json::Value SdnNdn::GetContentObjectJson(Ptr<const Data> contentObject)
{
    std::string responseString = SdnNdn::GetContentObjectString(contentObject);

    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(responseString, root))
    {
        std::cout << "Error while parsing json string \n";
    }

    return root;
}

}
}
}
