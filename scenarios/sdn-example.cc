// ndn-simple.cc

#include <cstdio>

#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"

#include "ns3-dev/ns3/ndnSIM/model/cs/content-store-impl.h"
#include "ns3-dev/ns3/ndnSIM/utils/trie/fifo-policy.h"

#include "../extensions/utils/sdncontroller.h"
#include "../extensions/utils/sdnp2phelper.h"

using namespace ns3;

void init(int argc, char *argv[])
{
    fprintf(stderr, "Launching Example Simulation\n");

    // setting default parameters for PointToPoint links and channels
    Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
    Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
    Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("20"));

    // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
    CommandLine cmd;
    cmd.Parse (argc, argv);

    // Creating nodes
    NodeContainer nodes;
    nodes.Create(19);

    ndn::fw::SDNController::clearGraphDb();

    // Connecting nodes using two links
    PointToPointHelper p2p;
    SDNP2PHelper sdnp2p(p2p);
    sdnp2p.SetDeviceAttribute("DataRate", "200kbps");


    sdnp2p.Install (nodes.Get (0), nodes.Get (2));
    sdnp2p.Install (nodes.Get (1), nodes.Get (2));
    sdnp2p.Install (nodes.Get (2), nodes.Get (3));

    sdnp2p.Install (nodes.Get (4), nodes.Get (3));
    sdnp2p.Install (nodes.Get (5), nodes.Get (4));
    sdnp2p.Install (nodes.Get (5), nodes.Get (1));
    sdnp2p.Install (nodes.Get (6), nodes.Get (5));
    sdnp2p.Install (nodes.Get (7), nodes.Get (0));
    sdnp2p.Install (nodes.Get (8), nodes.Get (2));
    sdnp2p.Install (nodes.Get (8), nodes.Get (5));
    sdnp2p.Install (nodes.Get (8), nodes.Get (7));
    sdnp2p.Install (nodes.Get (9), nodes.Get (4));

    sdnp2p.Install (nodes.Get (0), nodes.Get (10));
    sdnp2p.Install (nodes.Get (10), nodes.Get (11));
    sdnp2p.Install (nodes.Get (11), nodes.Get (3));
    sdnp2p.Install (nodes.Get (11), nodes.Get(12));
    sdnp2p.Install (nodes.Get (12), nodes.Get(13));
    sdnp2p.Install (nodes.Get (13), nodes.Get(14));
    sdnp2p.Install (nodes.Get (14), nodes.Get(15));
    sdnp2p.Install (nodes.Get (15), nodes.Get(13));
    sdnp2p.Install (nodes.Get (15), nodes.Get(16));
    sdnp2p.Install (nodes.Get (15), nodes.Get(17));
    sdnp2p.Install (nodes.Get (17), nodes.Get(14));
    sdnp2p.Install (nodes.Get (17), nodes.Get(18));
    sdnp2p.Install (nodes.Get (16), nodes.Get(10));

    /*
    for (int i = 0; i < 100; i++)
    {
        int rand1 = rand() % 50;
        int rand2 = rand() % 50;

        while (rand1 == rand2)
            rand2 = rand() % 50;
        //TODO: avoid duplicates
        sdnp2p.Install(nodes.Get(rand1), nodes.Get(rand2));
    }
    */
    // Install NDN stack on all nodes
    ndn::StackHelper ndnHelper;
    ndnHelper.SetDefaultRoutes (true);
    ndnHelper.SetContentStore("ns3::ndn::cs::Random");
    ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::SDNControlledStrategy", "EnableNACKs", "true");
    //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute", "EnableNACKs", "true");
    ndnHelper.Install(nodes);

    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    ndnGlobalRoutingHelper.Install(nodes);

    // Getting containers for the consumer/producer
    Ptr<Node> consumer1 = nodes.Get(0);
    Ptr<Node> consumer2 = nodes.Get(1);
    Ptr<Node> consumer3 = nodes.Get(6);
    Ptr<Node> consumer4 = nodes.Get(8);
    Ptr<Node> consumer5 = nodes.Get(18);
    Ptr<Node> consumer6 = nodes.Get(16);
    Ptr<Node> producer = nodes.Get(3);
    Ptr<Node> producer2 = nodes.Get(7);

    // Install NDN applications
    /*
    ndn::AppHelper helperSink ("Sink");
    ndn::AppHelper helperSource ("Source");
    ApplicationContainer sink1 = helperSink.Install(consumer1);
    ApplicationContainer sink2 = helperSink.Install(consumer2);
    ApplicationContainer source = helperSource.Install(producer);
    */

    ndn::AppHelper consumerHelper("ns3::ndn::StatisticsConsumer");
    // Consumer will request /prefix/0, /prefix/1, ...
    consumerHelper.SetPrefix("/itec/bunny_2s_480p_only/bunny_2s_100kbit/bunny_2s1.m4s");
    consumerHelper.SetAttribute("Frequency", StringValue("10"));
    ApplicationContainer sink1 = consumerHelper.Install(consumer1);
    ApplicationContainer sink2 = consumerHelper.Install(consumer2);
    ApplicationContainer sink3 = consumerHelper.Install(consumer3);
    ApplicationContainer sink4 = consumerHelper.Install(consumer4);
    //ApplicationContainer sink5 = consumerHelper.Install(consumer5);
    //ApplicationContainer sink6 = consumerHelper.Install(consumer6);

    // Producer
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    // Producer will reply to all requests starting with /prefix
    producerHelper.SetPrefix("/itec/bunny_2s_480p_only/bunny_2s_100kbit/bunny_2s1.m4s");
    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
    ApplicationContainer source = producerHelper.Install(producer);
    //ApplicationContainer source2 = producerHelper.Install(producer2);

    //ndn::fw::SDNController::AppFaceAddedToNode(consumer1);
    //ndn::fw::SDNController::AppFaceAddedToNode(consumer2);
    //ndn::fw::SDNController::AppFaceAddedToNode(producer);

    //Add /prefix origins to ndn::GlobalRouter
    std::string prefix = "/itec/bunny_2s_480p_only/bunny_2s_100kbit/bunny_2s1.m4s";
    ndnGlobalRoutingHelper.AddOrigins(prefix, producer);
    //ndnGlobalRoutingHelper.AddOrigins(prefix, producer2);
    ndn::fw::SDNController::AddOrigins(prefix, producer->GetId());
    //ndn::fw::SDNController::AddOrigins(prefix, producer2->GetId());
    // Calculate and install FIBs
    ndn::GlobalRoutingHelper::CalculateRoutes ();

    source.Start (Seconds (0.0)); // make source ready
    sink1.Start (Seconds (0.1)); // will send out Interest
    sink2.Start (Seconds (1.0)); // will send out Interest
    sink3.Start (Seconds(2.0));
    sink4.Start(Seconds(10));
    //sink5.Start(Seconds(3.0));
    //sink6.Start(Seconds(2.0));

    sink1.Stop(Seconds(60.0));
    sink2.Stop(Seconds(60.0));
    sink3.Stop(Seconds(60.0));
    sink4.Stop(Seconds(60.0));
    //sink5.Stop(Seconds(60.0));
    //sink6.Stop(Seconds(60.0));

    Simulator::Stop (Seconds (60.1));

    ndn::L3AggregateTracer::InstallAll ("aggregate-trace.txt", Seconds (0.5));
    ndn::L3RateTracer::InstallAll ("rate-trace.txt", Seconds (0.5));

    Simulator::Run ();
    Simulator::Destroy ();

    fprintf(stderr, "Simulation completed! Yeah! \n");
}

int main (int argc, char *argv[])
{
    init(argc, argv);

  return 0;
}

