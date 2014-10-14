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
#include "ns3/point-to-point-module.h"

#include "../extensions/randnetworks/networkgenerator.h"

#include "ns3/error-model.h"

#include "ns3/ndn-app.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include <stdlib.h>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "../extensions/randnetworks/ndnbritehelper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BriteExample");

int
main (int argc, char *argv[])
{
  // BRITE needs a configuration file to build its graph. By default, this
  // example will use the TD_ASBarabasi_RTWaxman.conf file. There are many others
  // which can be found in the BRITE/conf_files directory
  std::string confFile = "brite_test.conf";


  CommandLine cmd;
  cmd.AddValue ("confFile", "BRITE conf file", confFile);
  cmd.Parse (argc,argv);

  // Invoke the BriteTopologyHelper and pass config file

  ndn::StackHelper ndnHelper;
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy", "EnableNACKs", "true");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding", "EnableNACKs", "true");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "10000"); // all entities can store up to 1k chunks in cache (about 40MB)

  ndn::NetworkGenerator gen(confFile);

  fprintf(stderr, "Number of ASs = %d\n",gen.getNumberOfAS ());
  fprintf(stderr, "Number of ASNodes = %d\n",gen.getAllASNodes ().size ());
  fprintf(stderr, "Number of LeafNodes = %d\n",gen.getAllLeafNodes ().size ());

  PointToPointHelper *p2p = new PointToPointHelper;
  p2p->SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p->SetChannelAttribute ("Delay", StringValue ("2ms"));

  gen.randomlyPlaceNodes (5, "Server",ndn::NetworkGenerator::ASNode, p2p);
  gen.randomlyPlaceNodes (100, "Client",ndn::NetworkGenerator::LeafNode, p2p);

  ndnHelper.InstallAll();

  // Installing global routing interface on all routing nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.Install (gen.getAllASNodes ());
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true");

  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("4096"));

  NodeContainer server = gen.getCustomNodes ("Server");
  ndnGlobalRoutingHelper.Install (gen.getCustomNodes ("Server"));

  for(int i=0; i<server.size (); i++)
  {
    producerHelper.SetPrefix (std::string("/Server_" + boost::lexical_cast<std::string>(i) + "/layer0"));
    producerHelper.Install (Names::Find<Node>(std::string("Server_" + boost::lexical_cast<std::string>(i))));

    ndnGlobalRoutingHelper.AddOrigin(std::string("/Server_" + boost::lexical_cast<std::string>(i)),
                                      Names::Find<Node>(std::string("Server_" + boost::lexical_cast<std::string>(i))));
  }

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute ("Frequency", StringValue ("10")); // X interests a second
  consumerHelper.SetAttribute ("Randomize", StringValue ("uniform"));

  NodeContainer client = gen.getCustomNodes ("Client");
  ndnGlobalRoutingHelper.Install (gen.getCustomNodes ("Client"));
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true");

  for(int i=0; i<client.size (); i++)
  {
    consumerHelper.SetPrefix (std::string("/Server_" + boost::lexical_cast<std::string>(i%server.size ()) + "/layer0"));

    consumerHelper.Install (Names::Find<Node>(std::string("Client_" + boost::lexical_cast<std::string>(i))));
  }

    // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();
  ndn::GlobalRoutingHelper::CalculateRoutes ();


  // Run the simulator
  Simulator::Stop (Seconds (1800.0)); // 30 min
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
