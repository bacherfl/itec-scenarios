#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3/ndn-cs-tracer.h"

using namespace ns3;

class PcapWriter
{
public:
  PcapWriter (const std::string &file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile (file, std::ios::out, PcapHelper::DLT_PPP);
  }

  void TracePacket (Ptr<const Packet> packet)
  {
    static PppHeader pppHeader;
    pppHeader.SetProtocol (0x0077);

    m_pcap->Write (Simulator::Now (), pppHeader, packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};

void parseParameters(int argc, char* argv[])
{
  bool v0 = false, v1 = false, v2 = false;
  std::string top_path = "topologies/example2.top";

  CommandLine cmd;
  cmd.AddValue ("v0", "Prints all log messages >= LOG_DEBUG. (OPTIONAL)", v0);
  cmd.AddValue ("v1", "Prints all log messages >= LOG_INFO. (OPTIONAL)", v1);
  cmd.AddValue ("v2", "Prints all log messages. (OPTIONAL)", v2);
  cmd.AddValue ("top", "Path to the topology file. (OPTIONAL)", top_path);

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

  AnnotatedTopologyReader topologyReader ("", 25);
  topologyReader.SetFileName (top_path);
  topologyReader.Read();
}

int main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("Example2");

  parseParameters(argc, argv);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding");
  ndnHelper.InstallAll ();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // fetch src und dst nodes
  Ptr<Node> src1 = Names::Find<Node>("Src1");
  Ptr<Node> src2 = Names::Find<Node>("Src2");
  Ptr<Node> dst1 = Names::Find<Node>("Dst1");

  /*//install applications on nodes
  ndn::AppHelper helperSink ("Sink");
  ndn::AppHelper helperSource ("Source");
  ApplicationContainer source1 = helperSource.Install(src1);
  ApplicationContainer source2 = helperSource.Install(src2);
  ApplicationContainer sink = helperSink.Install(dst1);

  //Add /prefix origins to ndn::GlobalRouter
  std::string prefix = "/source";
  ndnGlobalRoutingHelper.AddOrigins(prefix, src1);
  ndnGlobalRoutingHelper.AddOrigins(prefix, src2);

  // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateRoutes ();
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  source1.Start (Seconds(0.0));
  source2.Start (Seconds(0.0));
  sink.Start (Seconds(0.1));
  */


#define ContentProvider

#ifdef ContentProvider
  ndn::AppHelper helperSource ("ContentProvider");
  helperSource.SetAttribute ("ContentPath", StringValue("/data/"));

#else
  ndn::AppHelper helperSource ("ns3::ndn::Producer");
  helperSource.SetAttribute ("PayloadSize", StringValue("1500"));

  //helperSource.SetAttribute("Freshness", TimeValue (Seconds (0)));
  helperSource.SetPrefix("/itec");
#endif


  ApplicationContainer source1 = helperSource.Install(src1);
#ifdef SRC2
  ApplicationContainer source2 = helperSource.Install(src2);
#endif

  ndn::AppHelper helperSink ("Sink");
  ApplicationContainer sink = helperSink.Install(dst1);

  std::string prefix = "/itec";
  ndnGlobalRoutingHelper.AddOrigins(prefix, src1);
#ifdef SRC2
  ndnGlobalRoutingHelper.AddOrigins(prefix, src2);
#endif

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

  //ndn::L3AggregateTracer::InstallAll ("aggregate-trace.txt", Seconds (0.5));

#ifdef PCAP
  PcapWriter trace ("ndn-simple-trace.pcap");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
         MakeCallback (&PcapWriter::TracePacket, &trace));
#endif

  source1.Start (Seconds(0.0));
#ifdef SRC2
  source2.Start (Seconds(0.0));
#endif
  sink.Start (Seconds(0.1));


  ndn::CsTracer::InstallAll ("cs-trace.txt", Seconds (1));

  NS_LOG_WARN("Simulation will be started!");

  Simulator::Stop (Seconds (100.0));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_WARN("Simulation completed!");
  return 0;
}

