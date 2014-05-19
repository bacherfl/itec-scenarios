#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include "../extensions/backgroundtraffic.h"

#include "../extensions/svc/svcadaptivestrategy.h"

#include <string>

using namespace ns3;

void parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "topologies/simple_ina.top";

  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("vN", "Disable all internal logging parameters, use NS_LOG instead", vN);
  cmd.AddValue ("top", "Path to the topology file. (OPTIONAL)", top_path);

  cmd.Parse (argc, argv);

  if (vN == false)
  {
    LogComponentEnableAll (LOG_ALL);

    if(!v2)
    {
      LogComponentDisableAll (LOG_LOGIC);
      LogComponentDisableAll (LOG_FUNCTION);
    }
    if(!v1 && !v2)
    {
      LogComponentDisableAll (LOG_INFO);
    }
    if(!v0 && !v1 && !v2)
    {
      LogComponentDisableAll (LOG_DEBUG);
    }
  }
  else
  {
    NS_LOG_UNCOND("Disabled internal logging parameters, using NS_LOG as parameter.");
  }

  AnnotatedTopologyReader topologyReader ("", 20);
  topologyReader.SetFileName (top_path);
  topologyReader.Read();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("PlayerTest");

  parseParameters(argc, argv);

  NodeContainer nodes;

  // fetch src und dst nodes
  Ptr<Node> contentDst = Names::Find<Node>("ContentDst");
  nodes.Add (contentDst);
  Ptr<Node> dummyDst = Names::Find<Node>("DummyDst");
  nodes.Add (dummyDst);
  Ptr<Node> adaptiveNode = Names::Find<Node>("AdaptiveNode");
  nodes.Add (adaptiveNode);
  Ptr<Node> simpleNode = Names::Find<Node>("SimpleNode");
  nodes.Add (simpleNode);
  Ptr<Node> dummySrc = Names::Find<Node>("DummySrc");
  nodes.Add (dummySrc);
  Ptr<Node> contentSrc = Names::Find<Node>("ContentSrc");
  nodes.Add (contentSrc);

  // Install NDN stack on all normal nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute",  "EnableNACKs", "true");
  ndnHelper.Install(nodes);

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  //multimedia traffic
  ndn::AppHelper dashRequesterHelper ("ns3::ndn::PlayerRequester");
  dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l/bbb-svc.264.mpd"));

  ApplicationContainer appContainer;
  appContainer = dashRequesterHelper.Install(contentDst);

  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/itec/bbb"));
  ApplicationContainer contentProvider = cProviderHelper.Install (contentSrc);

  ndnGlobalRoutingHelper.AddOrigins("/itec/bbb", contentSrc);
  ndnGlobalRoutingHelper.AddOrigins("/itec/sintel", contentSrc);

  contentProvider.Start (Seconds(0.0));
  appContainer.Start (Seconds(1.0));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds (3600.0)); //runs for 60 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}



