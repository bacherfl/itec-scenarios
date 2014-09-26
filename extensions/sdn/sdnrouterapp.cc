#include "sdnrouterapp.h"

#include "ns3/random-variable.h"
#include "sdn-ndn.h"

namespace ns3 {
namespace ndn {
namespace sdn {

NS_OBJECT_ENSURE_REGISTERED (SDNRouterApp);

SDNRouterApp::SDNRouterApp()
{
}

SDNRouterApp::~SDNRouterApp()
{
}

TypeId SDNRouterApp::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::ndn::sdn::SDNRouterApp")
        .SetParent<ndn::App> ()
        .AddConstructor<SDNRouterApp> ()
        ;
      return tid;
}

void SDNRouterApp::OnData(Ptr<const Data> contentObject)
{
    std::string prefix = contentObject->GetName().getPrefix(1, 0).toUri();

    //neighbour response
    if (prefix.compare(SdnNdn::NEIGHBOURS_PREFIX))
    {
        lastNeighbourUpdate = Simulator::Now().GetMilliSeconds();
    }
    else if (prefix.compare(SdnNdn::CONTROLLER_PREFIX))
    {
        lastControllerResponse = Simulator::Now().GetMilliSeconds();
        AddController(contentObject);
    }
}

void SDNRouterApp::AddController(Ptr<const Data> contentObject)
{
    uint8_t *buf = (uint8_t*) malloc(sizeof(uint8_t) * contentObject->GetPayload()->GetSize());
    contentObject->GetPayload()->CopyData(buf, contentObject->GetPayload()->GetSize());

    std::string responseStr = reinterpret_cast<char const*>(buf);
}

void SDNRouterApp::OnInterest(Ptr<const Interest> interest)
{
    std::string prefix = interest->GetName().getPrefix(1, 0).toUri();

    if (prefix.compare(SdnNdn::NEIGHBOURS_PREFIX) == 0)
    {
        Ptr<Node> node = GetNode();

        Json::Value responseContent = Json::Value(Json::objectValue);
        std::stringstream controllerIdStream;
        responseContent["id"] = node->GetId();
        //responseContent["seq"] = sequenceNumber++;

        Json::FastWriter writer;

        Ptr<Data> response = Create<Data> (Create<Packet> (writer.write(responseContent)));
        response->SetName(interest->GetName());
        response->SetTimestamp(Simulator::Now());
        //response->SetFreshness(Create<Time>(0));
        //response->GetPayload()->CopyData()
        uint8_t *buffer = (uint8_t*)(malloc (sizeof(uint8_t) * response->GetPayload()->GetSize()));

        response->GetPayload()->CopyData(buffer, response->GetPayload()->GetSize());

        std::string s (reinterpret_cast<char const*>(buffer));

        //std::cout << "Packet payload = " << s << "\n";

        m_face->ReceiveData(response);
        m_transmittedDatas(response, this, m_face);
    }
}

void SDNRouterApp::StopApplication()
{
}

void SDNRouterApp::StartApplication()
{
    // initialize ndn::App
    ndn::App::StartApplication ();
    DiscoverNeighbours();
    DiscoverController();
}

void SDNRouterApp::DiscoverNeighbours()
{
    Ptr<Node> node = GetNode();
    std::stringstream prefixStream;

    UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());

    prefixStream << SdnNdn::NEIGHBOURS_PREFIX;
    prefixStream << "/" << rand.GetValue(); //avoid caching; TODO: find better way to avoid caching of certain interests
    prefixStream << "/" << node->GetId();

    Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
    interest->SetName(Create<Name>(prefixStream.str()));

    interest->SetNonce (rand.GetValue ());
    interest->SetInterestLifetime (Seconds (1.0));

    //NS_LOG_DEBUG ("Sending Interest packet for " << prefixStream.str());

    // Call trace (for logging purposes)
    std::cout << "searching for neighbours \n";
    m_transmittedInterests (interest, this, m_face);

    m_face->ReceiveInterest (interest);

    //needs to be done only once => only the controller has to know about all connections
    //Simulator::Schedule(Seconds(2.0), &SDNRouterApp::DiscoverNeighbours, this);
}

void SDNRouterApp::DiscoverController()
{
    Ptr<Node> node = GetNode();
    std::stringstream prefixStream;

    prefixStream << SdnNdn::CONTROLLER_PREFIX;
    UniformVariable rand1 (0, std::numeric_limits<uint32_t>::max ());
    prefixStream << "/" << node->GetId();
    // Create and configure ndn::Interest
    Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
    interest->SetName(Create<Name>(prefixStream.str()));
    UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
    interest->SetNonce            (rand.GetValue ());
    interest->SetInterestLifetime (Seconds (1.0));

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);

    m_face->ReceiveInterest (interest);

    Simulator::Schedule(Seconds(1.0), &SDNRouterApp::RegisterAtController, this);
    Simulator::Schedule(Seconds(5.0), &SDNRouterApp::DiscoverController, this); //TODO: try out different intervals
}

void SDNRouterApp::RegisterAtController()
{

    //only register if there is at least one new known neighbour
    if (lastNeighbourUpdate > lastControllerRegistrationSent)
    {
        Ptr<Node> node = GetNode();
        std::stringstream prefixStream;
        prefixStream << SdnNdn::CONTROLLER_REGISTER
                     << "/" << node->GetId();

        Ptr<Interest> interest = Create<Interest>();
        interest->SetName(prefixStream.str());
        UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
        interest->SetNonce            (rand.GetValue ());
        interest->SetInterestLifetime (Seconds (1.0));

        // Call trace (for logging purposes)
        m_transmittedInterests (interest, this, m_face);

        m_face->ReceiveInterest (interest);
        //no need to reschedule here since scheduling is done in DiscoverController
        lastControllerRegistrationSent = Simulator::Now().GetMilliSeconds();
    }

}

} // namespace sdn
} // namespace sdn
} // namespace ndn
