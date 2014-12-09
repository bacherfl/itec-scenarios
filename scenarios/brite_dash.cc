
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/brite-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#include "../extensions/randnetworks/networkgenerator.h"

#include "ns3/error-model.h"

#include "ns3/ndn-app.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include <stdlib.h>
#include <string.h>

#include <boost/lexical_cast.hpp>
#include <ns3/log.h>

#include "../extensions/randnetworks/ndnbritehelper.h"

#include "../extensions/utils/parameterconfiguration.h"

using namespace ns3;

#define UNINITIALIZED -1.0

NS_LOG_COMPONENT_DEFINE ("BriteExample");

int
main (int argc, char *argv[])
{

  /*LogComponentEnableAll (LOG_ALL);
  LogComponentDisableAll (LOG_LOGIC);
  LogComponentDisableAll (LOG_FUNCTION);*/

  // BRITE needs a configuration file to build its graph.
  std::string confFile = "brite_configs/brite_low_bw.conf";
  std::string strategy = "bestRoute";
  std::string route = "all";
  std::string outputFolder = "output/";
  std::string conectivity = "medium";

  int totalLinkFailures = 0;

  double alpha = UNINITIALIZED;
  double x_dropping = UNINITIALIZED;
  double probing_traffic = UNINITIALIZED;
  double shift_threshold = UNINITIALIZED;
  double shift_traffic = UNINITIALIZED;
  double update_intervall = UNINITIALIZED;
  double max_layers = UNINITIALIZED;
  double reliability_threshold = UNINITIALIZED;

  CommandLine cmd;
  cmd.AddValue ("briteConfFile", "BRITE conf file", confFile);
  cmd.AddValue ("connectivity", "low, medium, high", conectivity);
  cmd.AddValue ("fw-strategy", "Forwarding Strategy", strategy);
  cmd.AddValue ("route", "defines if you use a single route or all possible routes", route);
  cmd.AddValue ("outputFolder", "defines specific output subdir", outputFolder);
  cmd.AddValue ("linkFailures", "defines number of linkfailures events", totalLinkFailures);

  cmd.AddValue ("ALPHA", "P_ALPHA", alpha);
  cmd.AddValue ("X_DROPPING", "P_X_DROPPING", x_dropping);
  cmd.AddValue ("PROBING_TRAFFIC", "P_PROBING_TRAFFIC", probing_traffic);
  cmd.AddValue ("SHIFT_THRESHOLD", "P_SHIFT_THRESHOLD", shift_threshold);
  cmd.AddValue ("SHIFT_TRAFFIC", "P_SHIFT_TRAFFIC", shift_traffic);
  cmd.AddValue ("UPDATE_INTERVALL", "P_UPDATE_INTERVALL", update_intervall);
  cmd.AddValue ("MAX_LAYERS", "P_MAX_LAYERS", max_layers);
  cmd.AddValue ("RELIABILITY_THRESHOLD", "P_RELIABILITY_THRESHOLD", reliability_threshold);

  cmd.Parse (argc,argv);

  if(alpha != UNINITIALIZED)
  {
    NS_LOG_DEBUG("alpha set to: " << alpha);
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("ALPHA", alpha);
  }

  if(x_dropping != UNINITIALIZED)
  {
    NS_LOG_DEBUG("x_dropping set to: " << x_dropping);
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("X_DROPPING", x_dropping);
  }

  if(probing_traffic != UNINITIALIZED)
  {
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("PROBING_TRAFFIC", probing_traffic);
    NS_LOG_DEBUG("probing_traffic set to: " << probing_traffic);
  }

  if(shift_threshold != UNINITIALIZED)
  {
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("SHIFT_THRESHOLD", shift_threshold);
    NS_LOG_DEBUG("shift_threshold set to: " << shift_threshold);
  }

  if(shift_traffic != UNINITIALIZED)
  {
    NS_LOG_DEBUG("shift_traffic set to: " << shift_traffic);
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("SHIFT_TRAFFIC", shift_traffic);
  }

  if(update_intervall != UNINITIALIZED)
  {
    NS_LOG_DEBUG("update_intervall set to: " << update_intervall);
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("UPDATE_INTERVALL", update_intervall);
  }

  if(max_layers != UNINITIALIZED)
  {
    NS_LOG_DEBUG("max_layers set to: " << max_layers);
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("MAX_LAYERS", max_layers);
  }

  if(reliability_threshold != UNINITIALIZED)
  {
    NS_LOG_DEBUG("reliability_threshold set to: " << reliability_threshold);
    ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("RELIABILITY_THRESHOLD_MIN", reliability_threshold);
  }

  // Invoke the BriteTopologyHelper and pass config file

  ndn::NetworkGenerator gen(confFile);

  fprintf(stderr, "Number of ASs = %d\n",gen.getNumberOfAS ());
  fprintf(stderr, "Number of ASNodes = %d\n",gen.getAllASNodes ().size ());
  fprintf(stderr, "Number of LeafNodes = %d\n",gen.getAllLeafNodes ().size ());

  PointToPointHelper *p2p = new PointToPointHelper;
  p2p->SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p->SetChannelAttribute ("Delay", StringValue ("2ms"));

  int min_bw_as = -1;
  int max_bw_as = -1;
  int min_bw_leaf = -1;
  int max_bw_leaf = -1;
  //int number_of_connections_between_as = -1;
  int additional_random_connections_as = -1;
  int additional_random_connections_leaf = - 1;

  if(confFile.find ("low_bw") != std::string::npos)
  {
    min_bw_as = 2000;
    max_bw_as = 4000;

    min_bw_leaf = 1000;
    max_bw_leaf = 2000;
  }
  else if(confFile.find ("medium_bw") != std::string::npos)
  {
    min_bw_as = 3000;
    max_bw_as = 5000;

    min_bw_leaf = 2000;
    max_bw_leaf = 4000;
  }
  else if (confFile.find ("high_bw") != std::string::npos)
  {
    min_bw_as = 4000;
    max_bw_as = 6000;

    min_bw_leaf = 3000;
    max_bw_leaf = 5000;
  }
  else if (confFile.find ("test") != std::string::npos)
  {
    min_bw_as = 40000;
    max_bw_as = 60000;

    min_bw_leaf = 30000;
    max_bw_leaf = 50000;
  }

  if(conectivity.compare ("low") == 0)
  {
    additional_random_connections_as = gen.getNumberOfAS () / 2;
    additional_random_connections_leaf = gen.getAllASNodesFromAS (0).size () / 3;
  }
  else if(conectivity.compare ("medium") == 0)
  {
    additional_random_connections_as = gen.getNumberOfAS ();
    additional_random_connections_leaf = gen.getAllASNodesFromAS (0).size () / 2;
  }
  else if (conectivity.compare ("high") == 0)
  {
    additional_random_connections_as = gen.getNumberOfAS () *2;
    additional_random_connections_leaf = gen.getAllASNodesFromAS (0).size ();
  }

  if(min_bw_as == -1 || max_bw_as == -1 || additional_random_connections_as == -1 || additional_random_connections_leaf == -1)
  {
    fprintf(stderr, "check szenario setting\n");
    exit(0);
  }

  //gen.randomlyAddConnectionsBetweenAllAS (number_of_connections_between_as,min_bw_as,max_bw_as,5,20);
  gen.randomlyAddConnectionsBetweenTwoAS (additional_random_connections_as,min_bw_as,max_bw_as,5,20);
  gen.randomlyAddConnectionsBetweenTwoNodesPerAS(additional_random_connections_leaf,min_bw_leaf,max_bw_leaf,5,20);

  //calculaute network connectivity be careful when u call this all nodes/edges are considered
  fprintf(stderr, "connectivity = %f\n",gen.calculateConnectivity());

  gen.randomlyPlaceNodes (10, "Server",ndn::NetworkGenerator::ASNode, p2p);
  gen.randomlyPlaceNodes (100, "Client",ndn::NetworkGenerator::LeafNode, p2p);

  double simTime = 300.0;

  for(int i = 0; i < totalLinkFailures; i++)
    gen.creatRandomLinkFailure(0, simTime, 0, simTime/10);

  ndn::StackHelper ndnHelper;

  if(strategy.compare ("perContentBased") == 0)
  {
    ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy", "EnableNACKs", "true");

    //disable token bucket
    //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy", "EnableNACKs", "true", "UseTokenBucket", "0");
  }
  else if(strategy.compare ("bestRoute") == 0)
  {
    ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
    ndnHelper.EnableLimits (true, Seconds (0.1), 4096, 50);
  }
  else if(strategy.compare("smartflooding") == 0 )
  {
    ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
    ndnHelper.EnableLimits (true, Seconds (0.1), 4096, 50);
  }
  else if(strategy.compare("broadcast") == 0 )
  {
    ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
    ndnHelper.EnableLimits (true, Seconds (0.1), 4096, 50);
  }
  else
  {
    fprintf(stderr,"Invalid Strategy. Exiting..\n ");
    exit(-1);
  }

  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "25000"); // all entities can store up to 1k chunks in cache (about 100MB)

  ndnHelper.InstallAll();

  // Installing global routing interface on all routing nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.Install (gen.getAllASNodes ());

  ndn::AppHelper producerHelper ("ContentProvider"); // use our content provider
  producerHelper.SetAttribute("ContentPath", StringValue("/data")); // set data path

  NodeContainer server = gen.getCustomNodes ("Server");
  ndnGlobalRoutingHelper.Install (gen.getCustomNodes ("Server"));

  for(int i=0; i<server.size (); i++)
  {
    producerHelper.SetPrefix (std::string("/Server_" + boost::lexical_cast<std::string>(i)));
    ApplicationContainer producer = producerHelper.Install (Names::Find<Node>(std::string("Server_" + boost::lexical_cast<std::string>(i))));
    producer.Start (Seconds(0.0)); // set start time of producer

    ndnGlobalRoutingHelper.AddOrigin(std::string("/Server_" + boost::lexical_cast<std::string>(i)),
                                      Names::Find<Node>(std::string("Server_" + boost::lexical_cast<std::string>(i))));
  }

  ndn::AppHelper consumerHelper ("ns3::ndn::PlayerRequester");
  consumerHelper.SetAttribute("EnableAdaptation", StringValue("0")); // for now use a simple congestion window on the client
  consumerHelper.SetAttribute ("CongestionWindowType", StringValue("static")); // no cong. window

  //set content layers
  ns3::ndn::ParameterConfiguration::getInstance ()->setParameter ("MAX_LAYERS", 3);

  NodeContainer client = gen.getCustomNodes ("Client");
  ndnGlobalRoutingHelper.Install (gen.getCustomNodes ("Client"));

  for(int i=0; i<client.size (); i++)
  {
    std::string mpdPath("/data/bunny_spatial_2s_set"+ boost::lexical_cast<std::string>(i%server.size ()) + "/bunny-spatial");
    std::string set("-server" + boost::lexical_cast<std::string>(i%server.size ()) + ".mpd");
    consumerHelper.SetAttribute ("MPD", StringValue(mpdPath.append(set).c_str()));

    ApplicationContainer consumer = consumerHelper.Install (Names::Find<Node>(std::string("Client_" + boost::lexical_cast<std::string>(i))));
    consumer.Start (Seconds(0.1));
    consumer.Stop (Seconds(simTime));

    //fprintf(stderr, "outputFolder=%s\n", std::string(outputFolder + "/aggregate-trace_"  + boost::lexical_cast<std::string>(i)).append(".txt").c_str());
    /*ndn::L3AggregateTracer::Install (Names::Find<Node>(std::string("Client_") + boost::lexical_cast<std::string>(i)),
                                     std::string(outputFolder + "/aggregate-trace_"  + boost::lexical_cast<std::string>(i)).append(".txt"), Seconds (simTime));


    ndn::AppDelayTracer::Install(Names::Find<Node>(std::string("Client_") + boost::lexical_cast<std::string>(i)),
                                 std::string(outputFolder +"/app-delays-trace_"  + boost::lexical_cast<std::string>(i)).append(".txt"));*/

  }

    // Calculate and install FIBs
  if(route.compare ("all") == 0)
  {
    ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();
  }
  else  if(route.compare ("single") == 0)
  {
    ndn::GlobalRoutingHelper::CalculateRoutes ();
  }
  else
  {
    fprintf(stderr,"Invalid Routing Policy. Exiting..\n ");
    exit(-1);
  }

  // Run the simulator
  Simulator::Stop (Seconds (simTime+0.1)); // 10 min
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
