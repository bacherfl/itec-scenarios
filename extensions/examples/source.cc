#include "source.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/packet.h"

#include "ns3-dev/ns3/ndn-app-face.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-data.h"

#include "ns3-dev/ns3/ndn-fib.h"
#include "ns3-dev/ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("Source");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Source);

// register NS-3 type
TypeId Source::GetTypeId ()
{
  static TypeId tid = TypeId ("Source").SetParent<ndn::App>().AddConstructor<Source>();
  return tid;
}

// Processing upon start of the application
void Source::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();

  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/source/data");

  // Creating FIB entry that ensures that we will receive incoming Interests
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);
}

// Processing when application is stopped
void Source::StopApplication ()
{
  ndn::App::StopApplication ();
}

void Source::SendInterest ()
{
}

// Callback that will be called when Interest arrives
void Source::OnInterest (Ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest (interest);
  
  NS_LOG_DEBUG ("Source: Received Interest packet for " << interest->GetName());

  // Create and configure ndn::Data and ndn::DataTail
  // (header is added in front of the packet, tail is added at the end of the 

  Ptr<ndn::Data> data = Create<ndn::Data> (Create<Packet> (1024));
  data->SetName (Create<ndn::Name> (interest->GetName ())); // data will have the same name as Interests

  NS_LOG_DEBUG ("Soruce: Sending Data packet for " << data->GetName ()); 

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);

  m_face->ReceiveData (data); 
}

// Callback that will be called when Data arrives
void Source::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_DEBUG ("Source: Should not received that packet: " << contentObject->GetName ());
}

} // namespace ns3
