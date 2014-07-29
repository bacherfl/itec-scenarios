#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/error-model.h"
#include "ns3/constant-position-mobility-model.h"

#include "ns3/ndn-app.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include <stdlib.h>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "../extensions/utils/idownloader.h"

using namespace ns3;

void parseParameters(int argc, char* argv[], int &num_providers, int &sim_duration)
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "validation_tops/as_sim.top";

  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("vN", "Disable all internal logging parameters, use NS_LOG instead", vN);
  //cmd.AddValue ("top", "Path to the topology file. (OPTIONAL)", top_path);
  cmd.AddValue ("num_provider", "Number of Content Providers. (Default 5)", num_providers);
  cmd.AddValue ("sim_duration", "Duration of Simulation in seconds. (Default 60)", sim_duration);

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
  NS_LOG_COMPONENT_DEFINE ("AS_SIMULATION");

  int num_providers = 5;
  int sim_duration = 60;

  parseParameters(argc, argv,num_providers,sim_duration);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;

  //Setup ASRouters
  NodeContainer as_routers;
  int nodeIndex = 0;
  std::string nodeNamePrefix = std::string("ASRouter");
  Ptr<Node> router = Names::Find<Node>(nodeNamePrefix + boost::lexical_cast<std::string>(nodeIndex++));
  while(router != NULL)
  {
    as_routers.Add (router);
    router = Names::Find<Node>(nodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
  }

  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy", "EnableNACKs", "true");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "32768"); // cache of 128MB
  ndnHelper.Install (as_routers);

  // setting default parameters for PointToPoint links and channels
   Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("20Mbps"));
   Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("2ms"));
   Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("50"));

  // Connecting nodes using two links
  PointToPointHelper p2p;

    //Creating content providers nodes and conect them to random AS nodes.
  NodeContainer providers;
  providers.Create (num_providers);

  UniformVariable random;
  int as_router_nr = 0;
  for(int i = 0; i < providers.size (); i++)
  {
    as_router_nr = random.GetInteger (0, as_routers.size ()-1);
    p2p.Install (providers.Get (i), as_routers.Get (as_router_nr));

    //Ptr<ConstantPositionMobilityModel> pos_prov = as_routers.Get (as_router_nr)->GetObject<ConstantPositionMobilityModel> ();
    //Ptr<ConstantPositionMobilityModel> pos_as = providers.Get (i)->GetObject<ConstantPositionMobilityModel> ();
    //pos_prov->SetPosition (Vector(i,0,0));
  }

  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1"); // ! Attention ! If set to 0, then MaxSize is infinite
  ndnHelper.Install (providers);

   // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();
  //gather all nodes in one containe


  //install provider applications
  ndn::AppHelper cProviderHelper ("ContentProvider");
  cProviderHelper.SetAttribute("ContentPath", StringValue("/data"));

  std::string prefix = "";
  for(int i = 0; i < providers.size (); i++)
  {
    if(i > 9)
    {
      NS_LOG_UNCOND("TOO MANY PROVIDERS, not enough content.");
      exit(-1);
    }

    prefix = std::string("/itec/bbb/bunny_svc_snr_2s_6l_set") + boost::lexical_cast<std::string>(i);
    cProviderHelper.SetAttribute("Prefix", StringValue(prefix.c_str ()));
    ApplicationContainer contentProvider = cProviderHelper.Install (providers.Get (i));
    contentProvider.Start (Seconds(0.0));

    //setup route
    ndnGlobalRoutingHelper.AddOrigins(prefix, providers.Get (i));
  }

  // Calculate and install FIBs
  //this is needed because otherwise On::Interest()-->createPITEntry will fail. Has no negative effect on the algorithm
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds(sim_duration)); //runs for 5 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}

