#include "sdnp2phelper.h"
#include "sdncontroller.h"
#include "ns3/core-module.h"

namespace ns3 {

using namespace ndn;

SDNP2PHelper::SDNP2PHelper(PointToPointHelper p2p):
    m_p2p(p2p)
{
}

NetDeviceContainer SDNP2PHelper::Install (NodeContainer c)
{
    return m_p2p.Install(c);
}


NetDeviceContainer SDNP2PHelper::Install (Ptr<Node> a, Ptr<Node> b)
{
    fw::SDNController::AddLink(a, b, channelAttributes, deviceAttributes);
    return m_p2p.Install(a, b);
}

NetDeviceContainer SDNP2PHelper::Install (Ptr<Node> a, std::string bName)
{
    return m_p2p.Install(a, bName);
}

NetDeviceContainer SDNP2PHelper::Install (std::string aName, Ptr<Node> b)
{
    return m_p2p.Install(aName, b);
}

NetDeviceContainer SDNP2PHelper::Install (std::string aNode, std::string bNode)
{
    return m_p2p.Install(aNode, bNode);
}

void SDNP2PHelper::SetDeviceAttribute (std::string name, std::string value)
{
    deviceAttributes[name] = value;
    m_p2p.SetDeviceAttribute(name, StringValue(value));
}

void SDNP2PHelper::SetChannelAttribute (std::string name, std::string value)
{
    channelAttributes[name] = value;
    m_p2p.SetChannelAttribute(name, StringValue(value));
}

}