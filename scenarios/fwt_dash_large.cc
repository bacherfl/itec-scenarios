#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/ndn-app.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include <stdlib.h>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "dash/iadaptationlogic.h"

#include "../extensions/utils/idownloader.h"

using namespace ns3;

void parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "validation_tops/fwt_dash_large.top";

  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("vN", "Disable all internal logging parameters, use NS_LOG instead", vN);
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
  NS_LOG_UNCOND("Using topology file " << top_path);
  topologyReader.SetFileName ("topologies/" + top_path);
  topologyReader.Read();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("Fwt_DASH_Large");

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

  int *client_map = (int*) calloc(streamers.size (), sizeof(int));
  for(int i = 0; i < streamers.size (); i++)
    client_map[i] = i;

  //random permutation
  srand(ns3::SeedManager::GetRun ());
  int swap, r;
  for(int i = 0; i < streamers.size (); i++)
  {
    r = rand()%streamers.size ();
    swap = client_map[i];
    client_map[i] = client_map[r];
    client_map[r] = swap;
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
    routers.Add (router);
    router = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  NodeContainer adaptiveNodes;
  nodeIndex = 0;
  nodeNamePrefix = std::string("AdaptiveNode");
  Ptr<Node> adaptiveNode = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(adaptiveNode != NULL)
  {
    adaptiveNodes.Add (adaptiveNode);
    adaptiveNode = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true"); // try without limits first...
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
  ndnHelper.Install (providers);
  ndnHelper.Install (streamers);

  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "64000"); // (about 256MB)
  ndnHelper.Install (routers);

  //change strategy for adaptive NODE
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
  ndnHelper.EnableLimits (true, Seconds(0.1), 4020, 50);
  /*ndnHelper.SetForwardingStrategy("ns3::ndn::fw::BestRoute::SVCLiveCountingStrategy",
                                  "EnableNACKs", "true", "LevelCount", "6");*/
  ndnHelper.Install (adaptiveNodes);

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

   //consumer
  //ndn::AppHelper dashRequesterHelper ("ns3::ndn::DashRequester");
  ndn::AppHelper dashRequesterHelper ("ns3::ndn::PlayerRequester");
  dashRequesterHelper.SetAttribute("EnableAdaptation", StringValue("1")); // uses SVCWindowsDownloaded
  dashRequesterHelper.SetAttribute("CongestionWindowType", StringValue("tcp")); // uses tcp like AIMD
  dashRequesterHelper.SetAttribute("AlogicType", IntegerValue(dashimpl::SimpleBuffer)); // uses buffer based alogic

  ApplicationContainer apps;
  // Spreading 10 Videos to 100 clients:
  // V1: 1 client ( 1 )
  // V2: 1 client ( 2 )
  // V3. 2 clients ( 3 )
  // V4. 2 clients ( 5 )
  // V5: 2 clients ( 7 )
  // V6: 4 clients ( 11 )
  // V7: 8 clients ( 19 )
  // V8: 16 clients ( 35 )
  // V9: 32 clients ( 67 )
  // V10: 33 clients ( 100 )

  int distribution[] = {1,2,3,5,7,11,19,35,67,100};

  for(int i=0; i < streamers.size (); i++)
    {
      if(i < distribution[0])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set0/bbb-svc.264_set0.mpd"));
      }
      else if(i < distribution[1])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set1/bbb-svc.264_set1.mpd"));
      }
      else if(i < distribution[2])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set2/bbb-svc.264_set2.mpd"));
      }
      else if(i < distribution[3])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set3/bbb-svc.264_set3.mpd"));
      }
      else if(i < distribution[4])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set4/bbb-svc.264_set4.mpd"));

      }
      else if(i < distribution[5])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set5/bbb-svc.264_set5.mpd"));
      }
      else if(i < distribution[6])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set6/bbb-svc.264_set6.mpd"));
      }
      else if(i < distribution[7])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set7/bbb-svc.264_set7.mpd"));
      }
      else if(i < distribution[8])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set8/bbb-svc.264_set8.mpd"));
      }
      else if(i < distribution[9])
      {
        dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_svc_snr_2s_6l_set9/bbb-svc.264_set9.mpd"));
      }

      apps.Add (dashRequesterHelper.Install(streamers.Get (client_map[i])));
    }

  //provider
  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/itec/bbb"));
  ApplicationContainer contentProvider = cProviderHelper.Install (providers);
  ndnGlobalRoutingHelper.AddOrigins("/itec/bbb", providers);

  contentProvider.Start (Seconds(0.0));

  //mean, upper-limit
  ns3::ExponentialVariable exp(12,30);
  Time stopTime = Seconds (1800.0);

  fprintf(stderr, "StartTime=");

  for (ApplicationContainer::Iterator i = apps.Begin (); i != apps.End (); ++i)
  {
    //int startTime = rand() % 30 + 1; //1-30
    //int startTime = exp.GetInteger () + 1;
    double startTime = exp.GetValue() + 1;

    //fprintf(stderr,"starttime = %d\n", startTime);
    //fprintf(stderr,"starttime = %f\n", startTime);
    ( *i)->SetStartTime(Time::FromDouble(startTime, Time::S));
    ( *i)->SetStopTime(stopTime);

    fprintf(stderr, "%f,", startTime);
  }
  fprintf(stderr, "\n");

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds(stopTime.GetSeconds ()+1)); //runs for 30 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}
