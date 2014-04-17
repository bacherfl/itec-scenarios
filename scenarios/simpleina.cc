#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include "../extensions/backgroundtraffic.h"

#include "../extensions/svc/svcadaptivestrategy.h"

using namespace ns3;

void parseParameters(int argc, char* argv[], bool &background_traffic)
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
  cmd.AddValue ("bg", "Enable background traffic. (OPTIONAL", background_traffic);


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
  } else {
    NS_LOG_UNCOND("Disabled internal logging parameters, using NS_LOG as parameter.");
  }
  AnnotatedTopologyReader topologyReader ("", 20);
  topologyReader.SetFileName (top_path);
  topologyReader.Read();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("SimpleINA");

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


  //adaptiveNodes.Add (adaptiveNode);
  normalNodes.Add (adaptiveNode); // just for svc-dash simmulation


  Ptr<Node> simpleNode = Names::Find<Node>("SimpleNode");
  normalNodes.Add (simpleNode);
  Ptr<Node> dummySrc = Names::Find<Node>("DummySrc");
  normalNodes.Add (dummySrc);
  Ptr<Node> contentSrc = Names::Find<Node>("ContentSrc");
  normalNodes.Add (contentSrc);

  // Install NDN stack on all normal nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate",  "EnableNACKs", "true");
  ndnHelper.EnableLimits (true, Seconds(0.1), 100, 4200);
  ndnHelper.Install(normalNodes);

  //change strategy for adaptive NODE
  ndnHelper.SetForwardingStrategy("ns3::ndn::fw::BestRoute::SVCCountingStrategy",
                                  "EnableNACKs", "true", "LevelCount", "3");
  ndnHelper.EnableLimits (false);
  ndnHelper.Install (adaptiveNodes);

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // enalbe background traffic?
  if (background_traffic)
  {
    NS_LOG_UNCOND("Background Traffic enabled");
    NodeContainer dummySrcNodes, dummyDstNodes;
    dummySrcNodes.Add(dummySrc);
    dummyDstNodes.Add(dummyDst);

    std::string dummy_prefix = "/dummy";

    // install background traffic
    BackgroundTraffic bgTrafficHelper(25.0, 50.0, dummy_prefix);
    bgTrafficHelper.Install(dummySrcNodes, dummyDstNodes, ndnGlobalRoutingHelper);
  }

  //multimedia traffic
  ndn::AppHelper dashRequesterHelper ("ns3::ndn::DashRequester");
  //dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_spatial_2s/bbb-svc.264.mpd"));
  dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l/bbb-svc.264.mpd"));
  dashRequesterHelper.SetAttribute ("BufferSize",UintegerValue(20));
  ApplicationContainer dashContainer = dashRequesterHelper.Install(contentDst);

/*
  ndn::AppHelper svcRequesterHelper ("ns3::ndn::SvcRequester");
  //svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/sintel_svc_spatial_2s/sintel-trailer-svc.264.mpd"));
  //svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/sintel_svc_snr_2s/sintel-trailer-svc.264.mpd"));
  svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l/bbb-svc.264.mpd"));
  svcRequesterHelper.SetAttribute ("BufferSize",UintegerValue(20));
  ApplicationContainer svcContainer = svcRequesterHelper.Install(contentDst);
*/

  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/itec/bbb"));
  ApplicationContainer contentProvider = cProviderHelper.Install (contentSrc);

  ndnGlobalRoutingHelper.AddOrigins("/itec/bbb", contentSrc);
  ndnGlobalRoutingHelper.AddOrigins("/itec/sintel", contentSrc);

  contentProvider.Start (Seconds(0.0));
  //svcContainer.Start (Seconds(1.0));
  dashContainer.Start (Seconds(1.0));
  //dummyProducer.Start (Seconds(0.0));
  //dummyConsumer.Start (Seconds(5.0));
  //dummyConsumer.Stop (Seconds(10.0));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds (1200.0)); //runs for 20 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}


