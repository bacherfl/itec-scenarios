#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include "../extensions/backgroundtraffic.h"

using namespace ns3;

void parseParameters(int argc, char* argv[], bool &background_traffic)
{
  bool v0 = false, v1 = false, v2 = false;

  std::string top_path = "topologies/simple_ina.top";

  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("top", "Path to the topology file. (OPTIONAL)", top_path);
  cmd.AddValue ("bg", "Enable background traffic. (OPTIONAL", background_traffic);


  cmd.Parse (argc, argv);

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
  AnnotatedTopologyReader topologyReader ("", 20);
  topologyReader.SetFileName (top_path);
  topologyReader.Read();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("CongestionTest");
  Time::SetResolution (Time::NS);

  bool  background_traffic = false;

  parseParameters(argc, argv, background_traffic);

  NodeContainer adaptiveNodes;
  NodeContainer normalNodes;

  // fetch src und dst nodes
  Ptr<Node> contentDst = Names::Find<Node>("ContentDst");
  normalNodes.Add (contentDst);
  Ptr<Node> dummyDst = Names::Find<Node>("DummyDst");
  normalNodes.Add (dummyDst);
  Ptr<Node> adaptiveNode = Names::Find<Node>("AdaptiveNode");
  adaptiveNodes.Add (adaptiveNode);
  Ptr<Node> simpleNode = Names::Find<Node>("SimpleNode");
  normalNodes.Add (simpleNode);
  Ptr<Node> dummySrc = Names::Find<Node>("DummySrc");
  normalNodes.Add (dummySrc);
  Ptr<Node> contentSrc = Names::Find<Node>("ContentSrc");
  normalNodes.Add (contentSrc);

  // Install NDN stack on all normal nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate",  "EnableNACKs", "true");
  ndnHelper.EnableLimits (true, Seconds(0.2), 4096, 100);
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // create two servers, one providing big buck bunny, the other one sintel
  // both files are original files, not split, no dash no svc nothing
  // the only point of this scenario is to test the congestion window

  ndn::AppHelper simpleFileHelper1 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper1.SetAttribute("URI", StringValue("/data/sintel-1024-surround.mp4"));
  ApplicationContainer fileContainer = simpleFileHelper1.Install(contentDst);




  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/data"));
  ApplicationContainer contentProvider = cProviderHelper.Install (contentSrc);

  ndnGlobalRoutingHelper.AddOrigins("/data", contentSrc);

  contentProvider.Start (Seconds(0.0));
  fileContainer.Start (Seconds(1.0));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds (1200.0)); //runs for 20 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}


