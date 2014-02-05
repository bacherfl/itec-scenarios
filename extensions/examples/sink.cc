#include "sink.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/packet.h"

#include "ns3-dev/ns3/ndn-app-face.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-data.h"

#include "ns3-dev/ns3/ndn-fib.h"
#include "ns3-dev/ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("Sink");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Sink);

// register NS-3 type
TypeId Sink::GetTypeId ()
{
  static TypeId tid = TypeId ("Sink").SetParent<ndn::App>().AddConstructor<Sink>();
  return tid;
}

// Processing upon start of the application
void Sink::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();

	SendInterest();
}

// Processing when application is stopped
void Sink::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

void Sink::SendInterest ()
{
  // Sending one Interest packet out
	 std::string n("/source/data");
	 //char* r_str = (char *) malloc(2*sizeof(char));
	 //n.append(random_numbers(r_str,2));

   Ptr<ndn::Name> prefix = Create<ndn::Name> (n.c_str());

	 // Create and configure ndn::Interest
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));

  NS_LOG_DEBUG ("Sink: Sending Interest packet for " << *prefix);
  
  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);
  m_face->ReceiveInterest (interest);
}

// Callback that will be called when Interest arrives
void Sink::OnInterest (Ptr<const ndn::Interest> interest)
{ 
  NS_LOG_DEBUG ("Sink: Should not receive Interest packet for " << interest->GetName());
}

// Callback that will be called when Data arrives
void Sink::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_DEBUG ("Sink: Receiving Data packet for " << contentObject->GetName());
  //SendInterest();
}

char* Sink::random_numbers(char * string, unsigned length)
{    
  unsigned int i;
  for (i = 0; i < length-1; ++i)
    {
      string[i] = rand() % (10) + 48;
    }

	string[length-1] = '\0';

	return string; 
}

} // namespace ns3
