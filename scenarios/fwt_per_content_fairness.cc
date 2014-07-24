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

#include <boost/lexical_cast.hpp>

#include "../extensions/utils/idownloader.h"

using namespace ns3;

void parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "validation_tops/fwt_per_content_fairness.top";

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
  NS_LOG_COMPONENT_DEFINE ("CongAvoidScenario");

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
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
  ndnHelper.EnableLimits (true, Seconds(0.1), 4200, 50);
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
  ndnHelper.Install (providers);
  ndnHelper.Install (streamers);

  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "64000"); // all entities can store up to 25k chunks in cache (about 100MB)
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
  ndnHelper.Install (routers);

  Ptr<Node> Router1 = Names::Find<Node>("Router1");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy", "EnableNACKs", "true");
  ndnHelper.EnableLimits (false);
  ndnHelper.Install (Router1);

  // Install NDN applications
  std::string prefix = "/data";

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix (prefix + "/layer0");
  consumerHelper.SetAttribute ("Frequency", StringValue ("100")); // X interests a second
  //consumerHelper.Install (streamers);

  Ptr<Node> ContentDst0 = Names::Find<Node>("ContentDst0");
  consumerHelper.SetPrefix (prefix+"c0/layer0");
  consumerHelper.SetAttribute ("Frequency", StringValue ("60")); // X interests a second
  consumerHelper.SetAttribute ("Randomize", StringValue ("uniform"));
  consumerHelper.Install (ContentDst0);

  Ptr<Node> ContentDst1 = Names::Find<Node>("ContentDst1");
  consumerHelper.SetPrefix (prefix +"c0/layer1");
  consumerHelper.SetAttribute ("Frequency", StringValue ("60")); // X interests a second
  consumerHelper.Install (ContentDst1);

  Ptr<Node> ContentDst2 = Names::Find<Node>("ContentDst2");
  consumerHelper.SetPrefix (prefix +"c0/layer2");
  consumerHelper.SetAttribute ("Frequency", StringValue ("60")); // X interests a second
  consumerHelper.Install (ContentDst2);

  Ptr<Node> ContentDst3 = Names::Find<Node>("ContentDst3");
  consumerHelper.SetPrefix (prefix+"c1/layer0");
  consumerHelper.SetAttribute ("Frequency", StringValue ("60")); // X interests a second
  consumerHelper.SetAttribute ("Randomize", StringValue ("uniform"));
  consumerHelper.Install (ContentDst3);

  Ptr<Node> ContentDst4 = Names::Find<Node>("ContentDst4");
  consumerHelper.SetPrefix (prefix +"c1/layer1");
  consumerHelper.SetAttribute ("Frequency", StringValue ("60")); // X interests a second
  consumerHelper.Install (ContentDst4);

  Ptr<Node> ContentDst5 = Names::Find<Node>("ContentDst5");
  consumerHelper.SetPrefix (prefix +"c1/layer2");
  consumerHelper.SetAttribute ("Frequency", StringValue ("60")); // X interests a second
  consumerHelper.Install (ContentDst5);


  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetPrefix (prefix);
  producerHelper.SetAttribute ("PayloadSize", StringValue("4096"));
  //producerHelper.Install (providers);

  Ptr<Node> ContentSrc0 = Names::Find<Node>("ContentSrc0");
  producerHelper.SetPrefix (prefix +"c0");
  producerHelper.Install (ContentSrc0);

  Ptr<Node> ContentSrc1 = Names::Find<Node>("ContentSrc1");
  producerHelper.SetPrefix (prefix +"c1");
  producerHelper.Install (ContentSrc1);

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  ndnGlobalRoutingHelper.AddOrigins(prefix, providers);
  ndnGlobalRoutingHelper.AddOrigins(prefix+"c0", providers);
  ndnGlobalRoutingHelper.AddOrigins(prefix+"c1", providers);

  // Calculate and install FIBs
  //this is needed because otherwise On::Interest()-->createPITEntry will fail. Has no negative effect on the algorithm
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds(300)); //runs for 5 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}


