#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include "../extensions/backgroundtraffic.h"

using namespace ns3;

void parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;

  std::string top_path = "topologies/simple_congestion_test.top";


  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("top", "Path to the topology file. (OPTIONAL)", top_path);

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

  parseParameters(argc, argv);


  // fetch src und dst nodes
  Ptr<Node> contentDst1 = Names::Find<Node>("ContentDst1");
  Ptr<Node> contentDst2 = Names::Find<Node>("ContentDst2");
  Ptr<Node> contentDst3 = Names::Find<Node>("ContentDst3");
  Ptr<Node> contentDst4 = Names::Find<Node>("ContentDst4");
  Ptr<Node> contentDst5 = Names::Find<Node>("ContentDst5");

  Ptr<Node> contentSrc1 = Names::Find<Node>("ContentSrc1");
  Ptr<Node> contentSrc2 = Names::Find<Node>("ContentSrc2");

  // Install NDN stack on all normal nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate",  "EnableNACKs", "true");
  ndnHelper.EnableLimits (true, Seconds(0.2), 100, 4200);
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // create two servers, one providing big buck bunny, the other one sintel
  // both files are original files, not split, no dash no svc nothing
  // the only point of this scenario is to test the congestion window

  ApplicationContainer fileContainer;

  ndn::AppHelper simpleFileHelper5 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper5.SetAttribute("URI", StringValue("/data/big_buck_bunny_480p_h264_copy.mov"));
  fileContainer = simpleFileHelper5.Install(contentDst5);
  fileContainer.Start (Seconds(5.0));

  ndn::AppHelper simpleFileHelper4 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper4.SetAttribute("URI", StringValue("/data/big_buck_bunny_480p_h264_copy.mov"));
  fileContainer = simpleFileHelper4.Install(contentDst4);
  fileContainer.Start (Seconds(4.0));

  ndn::AppHelper simpleFileHelper3 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper3.SetAttribute("URI", StringValue("/data/big_buck_bunny_480p_h264.mov"));
  fileContainer = simpleFileHelper3.Install(contentDst3);
  fileContainer.Start (Seconds(3.0));

  ndn::AppHelper simpleFileHelper2 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper2.SetAttribute("URI", StringValue("/data/sintel-1024-surround-copy.mp4"));
  fileContainer = simpleFileHelper2.Install(contentDst2);
  fileContainer.Start (Seconds(2.0));

  ndn::AppHelper simpleFileHelper1 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper1.SetAttribute("URI", StringValue("/data/sintel-1024-surround.mp4"));
  fileContainer = simpleFileHelper1.Install(contentDst1);
  fileContainer.Start (Seconds(1.0));


  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/data"));
  ApplicationContainer contentProvider1 = cProviderHelper.Install (contentSrc1);
  ApplicationContainer contentProvider2 = cProviderHelper.Install (contentSrc2);

  ndnGlobalRoutingHelper.AddOrigins("/data", contentSrc1);
  ndnGlobalRoutingHelper.AddOrigins("/data", contentSrc2);

  contentProvider1.Start (Seconds(0.0));
  contentProvider2.Start (Seconds(0.0));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds (800.0)); //runs for 20 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}


