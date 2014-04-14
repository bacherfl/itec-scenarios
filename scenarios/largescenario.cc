#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3/nstime.h"
#include <stdlib.h>
#include <string.h>

#include <boost/lexical_cast.hpp>

using namespace ns3;

void parseParameters(int argc, char* argv[], bool &background_traffic)
{
  bool v0 = false, v1 = false, v2 = false;

  std::string top_path = "topologies/large.top";

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
  NS_LOG_COMPONENT_DEFINE ("LargeScenario");

  bool  background_traffic = false;

  parseParameters(argc, argv, background_traffic);

  NodeContainer bbbStreamers;
  int nodeIndex = 0;
  std::string nodeNamePrefix("ContentDst");
  Ptr<Node> contentDst = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(contentDst != NULL)
  {
    bbbStreamers.Add (contentDst);
    contentDst =  Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  NodeContainer bbbProvider;
  nodeIndex = 1;
  nodeNamePrefix = std::string("ContentSrc");
  Ptr<Node> contentSrc = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(contentSrc != NULL)
  {
    bbbProvider.Add (contentSrc);
    contentSrc = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  NodeContainer routers;
  nodeIndex = 0;
  nodeNamePrefix = std::string("Router");
  Ptr<Node> router = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(router != NULL)
  {
    routers.Add (router);
    router = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }



  NodeContainer adaptiveNodes;
  nodeIndex = 0;
  nodeNamePrefix = std::string("AdaptiveNode");
  Ptr<Node> adaptiveNode = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(router != NULL)
  {
    adaptiveNodes.Add (adaptiveNode);
    adaptiveNode = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate",  "EnableNACKs", "true");
  ndnHelper.EnableLimits (true, Seconds(0.2), 100, 4200);
  ndnHelper.Install (bbbProvider);
  ndnHelper.Install (bbbStreamers);

  //routers become a special cache:
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "100"); // caches 10k chunks
  ndnHelper.Install (routers);


  //change strategy for adaptive NODE
  ndnHelper.SetForwardingStrategy("ns3::ndn::fw::BestRoute::SVCCountingStrategy", "EnableNACKs", "true");
  ndnHelper.EnableLimits (false);
  ndnHelper.Install (adaptiveNode);



  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

   //consumer
  /*ndn::AppHelper dashRequesterHelper ("ns3::ndn::DashRequester");
  dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_2s_480p_only/bunny_Desktop.mpd"));
  dashRequesterHelper.SetAttribute ("BufferSize",UintegerValue(20));
  ApplicationContainer dashContainer = dashRequesterHelper.Install(bbbStreamers);
  */

  ndn::AppHelper svcRequesterHelper ("ns3::ndn::SvcRequester");
  //svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/sintel_svc_spatial_2s/sintel-trailer-svc.264.mpd"));
  //svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/sintel_svc_snr_2s/sintel-trailer-svc.264.mpd"));
  svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_spatial_2s/bbb-svc.264.mpd"));
  svcRequesterHelper.SetAttribute ("BufferSize",UintegerValue(20));
  ApplicationContainer svcContainer = svcRequesterHelper.Install(bbbStreamers);


  //provider
  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/itec/bbb"));
  ApplicationContainer contentProvider = cProviderHelper.Install (bbbProvider);
  ndnGlobalRoutingHelper.AddOrigins("/itec/bbb", bbbProvider);

  contentProvider.Start (Seconds(0.0));

  srand (time(NULL));
  for (ApplicationContainer::Iterator i = svcContainer.Begin (); i != svcContainer.End (); ++i)
  {
    int startTime = rand() % 30 + 1; //1-30

    fprintf(stderr,"starttime = %d\n", startTime);
    ( *i)->SetStartTime(Time::FromInteger (startTime, Time::S));
  }

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds (1200.0)); //runs for 20 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}


