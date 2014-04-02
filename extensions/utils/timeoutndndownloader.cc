#include "timeoutndndownloader.h"

using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("TimeoutNDNDownloader");

TimeoutNDNDownloader::TimeoutNDNDownloader() : IDownloader()
{
  bytesToDownload = 0;
  chunk_number = 0;
  packets_received = 0;
  packets_timeout = 0;
  packets_inflight = 0;

  m_rtt = CreateObject<ndn::RttMeanDeviation> ();
}

bool TimeoutNDNDownloader::downloadBefore (Segment *s, int miliSeconds)
{
  return download(s);
}

bool TimeoutNDNDownloader::download (Segment *s)
{
  NS_LOG_FUNCTION(this);
  StartApplication ();

  bytesToDownload = s->getSize ();
  cur_chunk_uri = s->getUri();
  chunk_number = 0;
  packets_received = 0;
  packets_timeout = 0;
  packets_inflight = 0;

  // clear the last rtt information
  m_rtt->Reset();
  m_rtt->ClearSent();

  if(cur_chunk_uri.size () > 7 && cur_chunk_uri.substr (0,7).compare ("http://") == 0)
  {
    cur_chunk_uri = std::string("/").append(cur_chunk_uri.substr (7,cur_chunk_uri.length ()));
  }

  NS_LOG_FUNCTION(cur_chunk_uri << this);

  downloadChunk();
  return true;
}

void TimeoutNDNDownloader::downloadChunk ( )
{
  if(bytesToDownload > 0)
  {
    // get current RTO
    double rto = m_rtt->RetransmitTimeout().GetSeconds();
    rto = std::min<double>(rto, 1.0); // RTO must not be higher than 1 second here


    std::stringstream ss;
    ss << cur_chunk_uri << "/chunk_" << chunk_number;

    Ptr<ndn::Name> prefix = Create<ndn::Name> (ss.str ().c_str());

    // Create and configure ndn::Interest
    Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
    UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
    interest->SetNonce (rand.GetValue ());
    interest->SetName (prefix);
    interest->SetInterestLifetime (Seconds (rto)); // set interest life time equal to the event we are fireing
    // get the timeout event set up
    this->timeoutEvent =
      Simulator::Schedule(Seconds(rto), &TimeoutNDNDownloader::CheckRetrieveTimeout, this, chunk_number);


    // tell the RTO estimator that we sent out a packet
    m_rtt->SentSeq (SequenceNumber32 (chunk_number), 1);

    // increase packets_inflight
    packets_inflight++;

    //NS_LOG_FUNCTION("Sending Interest packet for " << *prefix << this);

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest (interest);
    chunk_number++;
  }
}


void TimeoutNDNDownloader::CheckRetrieveTimeout(int c_chunk_number)
{
  // timed out (else the event would have been canceled)
  OnTimeout(c_chunk_number);
}



void TimeoutNDNDownloader::OnTimeout (int c_chunk_number)
{
  fprintf(stderr, "Timeout on chunk %d\n", c_chunk_number);
  NS_LOG_FUNCTION("Timeout on chunk " << c_chunk_number << ". " << this);


  // reduce in_flight
  this->packets_inflight--;

  m_rtt->IncreaseMultiplier ();

  // re-request the packet
  this->chunk_number = c_chunk_number;
  this->downloadChunk();
}



void TimeoutNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  bytesToDownload -= contentObject->GetPayload()->GetSize ();
  packets_received++;
  // reduce packets_inflight by 1
  packets_inflight--;

  // find out what chunk this is
  std::string s =  contentObject->GetName().toUri().c_str();
  std::string segment_uri;
  unsigned int pos1 = s.find_last_of("chunk_");
  segment_uri = s.substr(0, pos1-6);

  s = s.substr(pos1+1);
  int c_chunk_number = atoi(s.c_str());


  // tell the RTO estimator that we received the packet with chunk_number
  m_rtt->AckSeq (SequenceNumber32 (c_chunk_number));

  // make sure to cancel the timeout event
  this->timeoutEvent.Cancel();


  if(bytesToDownload > 0)
    downloadChunk ();
  else
  {
    chunk_number = 0;
    NS_LOG_FUNCTION(std::string("Finally received segment: ").append(cur_chunk_uri.substr (0,cur_chunk_uri.find_last_of ("/chunk_"))) << this);
    notifyAll (); //notify observers
  }
}

// Processing upon start of the application
void TimeoutNDNDownloader::StartApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StartApplication ();
}

// Processing when application is stopped
void TimeoutNDNDownloader::StopApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StopApplication ();
}

void TimeoutNDNDownloader::setNodeForNDN (Ptr<Node> node)
{
  NS_LOG_FUNCTION(this);
  SetNode(node);
}
