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

NodeContainer parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;
  bool vN = false;

  std::string top_path = "abilene.top";

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

  return topologyReader.GetNodes ();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("ContentLookup");

  NodeContainer nodes = parseParameters(argc, argv);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;

  /*ndnHelper.EnableLimits (true, Seconds(0.1), 4020, 50);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Rate", "EnableNACKs", "true");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "1000"); // all entities can store up to 1k chunks in cache (about 4MB)
 */

  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Nacks::PerContentBasedLayerStrategy", "EnableNACKs", "true");
  ndnHelper.EnableLimits (false);
  ndnHelper.SetContentStore ("ns3::ndn::cs::Stats::Lru","MaxSize", "64000"); // all entities can store up to 25k chunks in cache (about 100MB)

  ndnHelper.Install (nodes);

  // Install NDN applications
  std::string prefix = "/data/";

  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("4096"));
  Ptr<Node> curNode;
  for(int i = 0; i < nodes.size (); i++)
  {
    curNode = nodes.Get (i);
    producerHelper.SetPrefix (prefix + Names::FindName(curNode));
    producerHelper.Install (curNode);
  }

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix (prefix + "NYCMng" + "/layer0");
  consumerHelper.SetAttribute ("Frequency", StringValue ("1000")); // X interests a second
  consumerHelper.SetAttribute ("Randomize", StringValue ("uniform"));
  consumerHelper.Install(Names::Find<Node>("STTLng"));

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  for(int i = 0; i < nodes.size (); i++)
  {
    curNode = nodes.Get (i);
    ndnGlobalRoutingHelper.AddOrigins(prefix + Names::FindName(curNode), curNode);
  }

  /*
  // installs a second privder for (prefix + "NYCMng") at HSTNng.
  producerHelper.SetPrefix (prefix + "NYCMng");
  producerHelper.Install(Names::Find<Node>("HSTNng"));
  ndnGlobalRoutingHelper.AddOrigins(prefix + "NYCMng", Names::Find<Node>("HSTNng"));
  */

  // Calculate and install FIBs
  //this is needed because otherwise On::Interest()-->createPITEntry will fail. Has no negative effect on the algorithm
  //ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();
  ndn::GlobalRoutingHelper::CalculateRoutes();

  NS_LOG_UNCOND("Simulation will be started!");

  Simulator::Stop (Seconds(300)); //runs for 5 min.
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND("Simulation completed!");
  return 0;
}


