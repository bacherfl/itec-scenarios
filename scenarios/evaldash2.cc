
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

#include "../extensions/utils/idownloader.h"

using namespace ns3;

void parseParameters(int argc, char* argv[], std::string& cwnd, std::string& cache_size, int& bgTrafficClients)
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "dasheval_100clients.top";

  cache_size = "10000";
  std::string strBgTrafficClients = "0";

  cwnd = "tcp";

  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("vN", "Disable all internal logging parameters, use NS_LOG instead", vN);
  cmd.AddValue ("top", "Path to the topology file. (OPTIONAL)", top_path);
  cmd.AddValue ("cwnd", "Type of congestion window to be used (either tcp (default) or static) (OPTIONAL)", cwnd);
  cmd.AddValue ("cacheSize", "Size of caches for network nodes", cache_size);
  cmd.AddValue ("bgTrafficClients", "Number of additional clients causing background traffic", strBgTrafficClients);

  cmd.Parse (argc, argv);

  try {
  // parse strBgTrafficClients to number
    bgTrafficClients = boost::lexical_cast<int>(strBgTrafficClients);
  }
  catch  ( const boost::bad_lexical_cast &exc )
  {
    bgTrafficClients = 0;
  }

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
  NS_LOG_COMPONENT_DEFINE ("DashEvaluationScenario2");

  std::string cwnd("tcp");
  std::string cache_size("10000");
  int bgTrafficClients = 0;

  parseParameters(argc, argv, cwnd, cache_size, bgTrafficClients);

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

  // print client map
  fprintf(stderr, "client_map=");
  for (int i = 0; i < streamers.size(); i++)
  {
      fprintf(stderr, "%d,", client_map[i]);
  }
  fprintf(stderr, "\n");


  NodeContainer fileRequester;
  nodeIndex = 0;
  nodeNamePrefix = std::string("FileRequester");
  contentDst = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  while(contentDst != NULL)
  {
    fileRequester.Add (contentDst);
    contentDst =  Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }
  NS_LOG_UNCOND("Dash-Clients: " << streamers.size() << ", FileRequesters: " << fileRequester.size());



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

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true"); // try without limits first...
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate",  "EnableNACKs", "true");
  //ndnHelper.EnableLimits (true, Seconds(0.2), 100, 4200);
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
  ndnHelper.Install (providers);
  ndnHelper.Install (streamers);
  ndnHelper.Install (fileRequester);

  // set cache size on routers
  NS_LOG_UNCOND("Cache-Size for all routers is set to " << cache_size);
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", cache_size);
  ndnHelper.Install (routers);

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  //consumer
  //ndn::AppHelper dashRequesterHelper ("ns3::ndn::DashRequester");
  ndn::AppHelper dashRequesterHelper ("ns3::ndn::PlayerRequester");
  //dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/artifical1/artifical1.mpd"));

  dashRequesterHelper.SetAttribute("EnableAdaptation", StringValue("0"));
  dashRequesterHelper.SetAttribute("CongestionWindowType", StringValue(cwnd));

  ApplicationContainer fileContainer;

  ndn::AppHelper simpleFileHelper0 ("ns3::ndn::SimpleFileRequester");
  simpleFileHelper0.SetAttribute("URI", StringValue("/itec/bbb/sintel-1024-surround-copy.mp4"));
  fileContainer = simpleFileHelper0.Install(fileRequester);

  /*
  ndn::AppHelper svcRequesterHelper ("ns3::ndn::SvcRequester");
  //svcRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s/bbb-svc.264.mpd"));
  svcRequesterHelper.SetAttribute ("BufferSize",UintegerValue(20));
  */
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

  for (int i=0; i < streamers.size (); i++)
  {
    if(i < distribution[0])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set0/bunny-spatial-set0.mpd"));
   }
    else if(i < distribution[1])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set1/bunny-spatial-set1.mpd"));
    }
    else if(i < distribution[2])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set2/bunny-spatial-set2.mpd"));
    }
    else if(i < distribution[3])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set3/bunny-spatial-set3.mpd"));
    }
    else if(i < distribution[4])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set4/bunny-spatial-set4.mpd"));
    }
    else if(i < distribution[5])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set5/bunny-spatial-set5.mpd"));
    }
    else if(i < distribution[6])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set6/bunny-spatial-set6.mpd"));
    }
    else if(i < distribution[7])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set7/bunny-spatial-set7.mpd"));
    }
    else if(i < distribution[8])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set8/bunny-spatial-set8.mpd"));
    }
    else if(i < distribution[9])
    {
      dashRequesterHelper.SetAttribute ("MPD",StringValue("/data/bunny_spatial_2s_set9/bunny-spatial-set9.mpd"));
    }

    apps.Add (dashRequesterHelper.Install(streamers.Get (client_map[i])));
  }

  //provider
  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));
  cProviderHelper.SetAttribute("Prefix", StringValue("/itec/bbb"));
  ApplicationContainer contentProvider = cProviderHelper.Install (providers);
  ndnGlobalRoutingHelper.AddOrigins("/itec", providers);

  contentProvider.Start (Seconds(0.0));

  //srand (1);
  // Exponential Distribution: mean, upper-limit
  ns3::ExponentialVariable exp(12,30);
  Time stopTime = Seconds (3600.0);


  fprintf(stderr, "StartTimes(DASH)=");

  for (ApplicationContainer::Iterator i = apps.Begin (); i != apps.End (); ++i)
  {
    double startTime = exp.GetValue() + 1;

    ( *i)->SetStartTime(Time::FromDouble(startTime, Time::S));
    ( *i)->SetStopTime(stopTime);

    fprintf(stderr, "%f,", startTime);
  }
  fprintf(stderr, "\n");


  int cnt = 0;
  fprintf(stderr, "StartTimes(File)=");
  for (ApplicationContainer::Iterator i = fileContainer.Begin (); i != fileContainer.End (); ++i)
  {
    if (cnt < bgTrafficClients)
    {
      double startTime = exp.GetValue() + 1;

      ( *i)->SetStartTime(Time::FromDouble(startTime, Time::S));
      ( *i)->SetStopTime(stopTime);

      fprintf(stderr, "%f,", startTime);
    } else {
      (*i)->SetAttribute("URI", StringValue(""));
    }
    cnt++;
  }
  fprintf(stderr, "\n");

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds(stopTime.GetSeconds ()+1)); //runs for 60 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}
