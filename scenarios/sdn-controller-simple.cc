// sdn-controller-simple.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating nodes
  Ptr<Node> node0 = CreateObject<Node> ();
  Ptr<Node> node1 = CreateObject<Node> ();
  Ptr<Node> node2 = CreateObject<Node> ();
  Ptr<Node> node3 = CreateObject<Node> ();

  //Connect nodes
  PointToPointHelper p2pHelper;
  p2pHelper.Install(node0, node2);
  p2pHelper.Install(node0, node3);
  p2pHelper.Install(node2, node1);
  p2pHelper.Install(node3, node1);


  // Install CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.SetForwardingStrategy("ns3::ndn::fw::SDNForwardingStrategy");
  ccnxHelper.InstallAll ();

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Installing applications

  // SDNRouterApp
  ndn::AppHelper consumerHelper ("ns3::ndn::sdn::SDNRouterApp");
  ApplicationContainer sdnRouter1 = consumerHelper.Install (node0);
  ApplicationContainer sdnRouter12 = consumerHelper.Install (node2);
  ApplicationContainer sdnRouter3 = consumerHelper.Install (node3);
  sdnRouter1.Start (Seconds (2.0));
  sdnRouter12.Start (Seconds (5.0));
  sdnRouter3.Start (Seconds (1.0));


  //Controller
  ndn::AppHelper controllerHelper ("ns3::ndn::sdn::SDNController");
  ApplicationContainer controller = controllerHelper.Install(node1);
  controller.Start (Seconds (6.0));

  Ptr<Node> allNodes[4] = {node0, node1, node2, node3};

  for (int i = 0; i < 4; i++)
  {
      ndnGlobalRoutingHelper.AddOrigin("/neighbours", allNodes[i]);
      ndnGlobalRoutingHelper.AddOrigin("/router", allNodes[i]);
  }

  ndnGlobalRoutingHelper.AddOrigin("/controller", node1);


  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop (Seconds (15.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
