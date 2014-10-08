#include "sdncontroller.h"
#include <stdio.h>
#include "sdn-ndn.h"

#include "ns3/random-variable.h"

namespace ns3 {
namespace ndn {
namespace sdn{

using namespace std;

NS_OBJECT_ENSURE_REGISTERED (SDNController);

SDNController::SDNController()
{
}

SDNController::~SDNController()
{
}

TypeId SDNController::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::ndn::sdn::SDNController")
        .SetParent<ndn::App> ()
        .AddConstructor<SDNController> ()
        ;
      return tid;
}

void SDNController::StartApplication()
{
    App::StartApplication();
    sdnInterestHandler = Create<SDNInterestHandler> (this);

    Ptr<ndn::Name> prefix = Create<ndn::Name> ("/");
    //Creating FIB entry that ensures that we will receive incoming Interests
    Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();
    Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);
}

void SDNController::StopApplication()
{
    App::StopApplication();
}

void SDNController::OnInterest(Ptr<const Interest> interest)
{
    std::cout << "CONTROLLER: Received Interest " << interest->GetName().toUri() << "\n";
    App::OnInterest(interest);
    Ptr<Data> response = sdnInterestHandler->ProcessInterest(interest);

    if (response != NULL)
    {
        m_face->ReceiveData(response);
        m_transmittedDatas(response, this, m_face);
    }

    //if a registration has been received, the neighbours of the new router are queried
    if (interest->GetName().toUri().find(SdnNdn::CONTROLLER_REGISTER) != std::string::npos)
    {
        SendNeighbourRequestToRouter(interest->GetName().getPostfix(1,0).toUri());
    }
}

void SDNController::SendNeighbourRequestToRouter(std::string routerId)
{
    std::cout << "sending neighbour request to router " << routerId << "\n";
    std::stringstream prefixStream;
    prefixStream << SdnNdn::CONTROLLER_NEIGHBOUR_REQUEST
                 << routerId;

    Ptr<Interest> interest = Create<Interest>();
    interest->SetName(prefixStream.str());
    UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
    interest->SetNonce            (rand.GetValue ());
    interest->SetInterestLifetime (Seconds (1.0));

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);

    m_face->ReceiveInterest (interest);
}

void SDNController::OnData(Ptr<const Data> contentObject)
{
    App::OnData(contentObject);
    std::string prefix = contentObject->GetName().toUri();

    if (prefix.find(SdnNdn::CONTROLLER_NEIGHBOUR_REQUEST) != std::string::npos)
    {
        Json::Value root = SdnNdn::GetContentObjectJson(contentObject);

        if (!root.isNull())
        {
            Json::Value neighbours = root["neighbours"];
            if (neighbours.isArray())
            {
                for (int i = 0; i < neighbours.size(); i++)
                {
                    Json::Value neighbour = neighbours[i];
                    cout << neighbour.get("id", "").asString() << ", RTT=" << neighbour.get("rtt", 0).asInt() << "\n";
                }
            }

        }
    }

}

} //namesapce sdn
} //namespace ndn
} //namespace ns-3
