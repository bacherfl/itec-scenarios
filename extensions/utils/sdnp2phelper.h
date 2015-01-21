#ifndef SDNP2PHELPER_H
#define SDNP2PHELPER_H

#include "ns3/point-to-point-helper.h"
#include <map>

namespace ns3 {

class SDNP2PHelper
{
public:
    SDNP2PHelper(PointToPointHelper p2p);
    virtual ~SDNP2PHelper () {}

    /**
     * \param c a set of nodes
     *
     * This method creates a ns3::PointToPointChannel with the
     * attributes configured by PointToPointHelper::SetChannelAttribute,
     * then, for each node in the input container, we create a
     * ns3::PointToPointNetDevice with the requested attributes,
     * a queue for this ns3::NetDevice, and associate the resulting
     * ns3::NetDevice with the ns3::Node and ns3::PointToPointChannel.
     */
    NetDeviceContainer Install (NodeContainer c);

    /**
     * \param a first node
     * \param b second node
     *
     * Saves you from having to construct a temporary NodeContainer.
     * Also, if MPI is enabled, for distributed simulations,
     * appropriate remote point-to-point channels are created.
     */
    NetDeviceContainer Install (Ptr<Node> a, Ptr<Node> b);

    /**
     * \param a first node
     * \param bName name of second node
     *
     * Saves you from having to construct a temporary NodeContainer.
     */
    NetDeviceContainer Install (Ptr<Node> a, std::string bName);

    /**
     * \param aName Name of first node
     * \param b second node
     *
     * Saves you from having to construct a temporary NodeContainer.
     */
    NetDeviceContainer Install (std::string aName, Ptr<Node> b);

    /**
     * \param aNode Name of first node
     * \param bNode Name of second node
     *
     * Saves you from having to construct a temporary NodeContainer.
     */
    NetDeviceContainer Install (std::string aNode, std::string bNode);

    /**
     * Set an attribute value to be propagated to each NetDevice created by the
     * helper.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * Set these attributes on each ns3::PointToPointNetDevice created
     * by PointToPointHelper::Install
     */
    void SetDeviceAttribute (std::string name, std::string value);

    /**
     * Set an attribute value to be propagated to each Channel created by the
     * helper.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * Set these attribute on each ns3::PointToPointChannel created
     * by PointToPointHelper::Install
     */
    void SetChannelAttribute (std::string name, std::string value);

private:
    PointToPointHelper m_p2p;
    std::map<std::string, std::string > deviceAttributes;
    std::map<std::string, std::string > channelAttributes;

};

}

#endif // SDNP2PHELPER_H
