#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/error-model.h"

#include "ns3/ndn-app.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include <stdlib.h>
#include <string.h>

#include "dash/iadaptationlogic.h"

#include <boost/lexical_cast.hpp>

#include "../extensions/utils/idownloader.h"

using namespace ns3;

void parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "validation_tops/fwt_dash_simple.top";

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
  } else {
    NS_LOG_UNCOND("Disabled internal logging parameters, using NS_LOG as parameter.");
  }
  AnnotatedTopologyReader topologyReader ("", 10);
  NS_LOG_UNCOND("Using topology file " << top_path);
  topologyReader.SetFileName ("topologies/" + top_path);
  topologyReader.Read();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("fwt_dash_simple");

  parseParameters(argc, argv);

  NodeContainer streamers;
  int nodeIndex = 0;
  std::string nodeNamePrefix("ContentDst");
  Ptr<Node> contentDst = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(contentDst != NULL)
  {
    streamers.Add (contentDst);
    contentDst =  Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  NodeContainer providers;
  nodeIndex = 0;
  nodeNamePrefix = std::string("ContentSrc");
  Ptr<Node> contentSrc = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(contentSrc != NULL)
  {
    providers.Add (contentSrc);
    contentSrc = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  NodeContainer routers;
  nodeIndex = 0;
  nodeNamePrefix = std::string("Router");
  Ptr<Node> router = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(router != NULL)
  {
    if(nodeIndex == 1)
    {
      nodeIndex++;
      continue;
    }
    routers.Add (router);
    router = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
  ndnHelper.Install (providers);
  ndnHelper.Install (streamers);

  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "64000"); // all entities can store up to 25k chunks in cache (about 100MB)
  ndnHelper.Install (routers);

  Ptr<Node> Router1 = Names::Find<Node>("Router1");
  //ndnHelper.SetForwardingStrategy("ns3::ndn::fw::BestRoute::SVCLiveCountingStrategy","EnableNACKs", "true", "LevelCount", "6");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy::PerOutFaceLimits",
                                   "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true", "PrefixNameComponentIndex", "2");
  ndnHelper.EnableLimits (true, Seconds(0.1), 4020, 50);
  ndnHelper.Install (Router1);

  // Install NDN applications
  ndn::AppHelper dashRequesterHelper ("ns3::ndn::PlayerRequester");
  dashRequesterHelper.SetAttribute("EnableAdaptation", StringValue("1"));
  dashRequesterHelper.SetAttribute("CongestionWindowType", StringValue("tcp"));
  dashRequesterHelper.SetAttribute("AlogicType", IntegerValue(dashimpl::SimpleBuffer));
  //dashRequesterHelper.SetAttribute("AlogicType", IntegerValue(dashimpl::LayerdSieber));

  ApplicationContainer apps;
  for(int i=0; i < streamers.size (); i++)
  {
    dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set"+ boost::lexical_cast<std::string>(i%10)+"/bbb-svc.264_set"+ boost::lexical_cast<std::string>(i%10) +".mpd"));
    apps.Add (dashRequesterHelper.Install (streamers.Get (i)));
  }

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/itec/bbb"));
  ApplicationContainer contentProvider = cProviderHelper.Install (providers);
  ndnGlobalRoutingHelper.AddOrigins("/itec/bbb", providers);

  contentProvider.Start (Seconds(0.0));

  Time stopTime = Seconds (3600.0);

  for (ApplicationContainer::Iterator i = apps.Begin (); i != apps.End (); ++i)
  {
    ( *i)->SetStartTime(Seconds(0));
    ( *i)->SetStopTime(stopTime);
  }

  // Calculate and install FIBs
  //this is needed because otherwise On::Interest()-->createPITEntry will fail. Has no negative effect on the algorithm
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds(3601)); //runs for 5 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}

