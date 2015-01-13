#include "sdncontroller.h"

namespace ns3 {
namespace ndn {

using namespace boost::tuples;
using namespace std;

SDNController::SDNController()
{
    globalRouter = Create<GlobalRouter>();
}

void SDNController::CalculateRoutesForPrefix(Ptr<Node> start, const std::string &prefix)
{
    //find origin node for prefix
    vector<Ptr<Node> > origins = contentOrigins[prefix];
    if (origins.size() > 0)
    {
        Ptr<Node> origin = origins.get(0);


    }
}

void SDNController::AddOrigins(std::string &prefix, Ptr<Node> producer)
{
    contentOrigins[prefix].add(producer);
}

void SDNController::PushRoute(Route route)
{

}

void SDNController::AddLink(Ptr<Node> a,
                            Ptr<Node> b,
                            std::map<std::string, AttributeValue& > channelAttributes,
                            std::map<std::string, AttributeValue& > deviceAttributes)
{
    incidencyList.insert(make_tuple(a, b, channelAttributes, deviceAttributes));
}

void SDNController::RequestForUnknownPrefix(std::string &prefix)
{

}

void SDNController::NodeReceivedNackOnFace(Ptr<Node>, Ptr<Face>)
{

}

}
}
