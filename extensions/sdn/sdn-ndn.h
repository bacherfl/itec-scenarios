#ifndef SDNNDN_H
#define SDNNDN_H

#include "ns3-dev/ns3/ndn-data.h"
#include <string>
#include <jsoncpp/json/json.h>

namespace ns3 {
namespace ndn {
namespace sdn {

/**
    Utility class containing constants and helper functions
*/
class SdnNdn {
public:
    static const std::string CONTROLLER_PREFIX;
    static const std::string NEIGHBOURS_PREFIX;
    static const std::string REGISTER_PREFIX;
    static const std::string CONTROLLER_REGISTER;
    static const std::string CONTROLLER_NEIGHBOUR_REQUEST;

    static std::string GetContentObjectString(Ptr<const Data> contentObject);
    static Json::Value GetContentObjectJson(Ptr<const Data> contentObject);
};


}
}
}

#endif // SDNNDN_H

