#include "sdncontroller.h"
#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace sdn{

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
    std::cout << "Received Interest " << interest->GetName().toUri() << "\n";
    App::OnInterest(interest);
    Ptr<Data> response = sdnInterestHandler->ProcessInterest(interest);

    if (response != NULL)
    {
        m_face->ReceiveData(response);
        m_transmittedDatas(response, this, m_face);
    }
}

void SDNController::OnData(Ptr<const Data> contentObject)
{
    App::OnData(contentObject);
}

} //namesapce sdn
} //namespace ndn
} //namespace ns-3
