#include "simplendndownloader.h"

using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("SimpleNDNDownloader");

SimpleNDNDownloader::SimpleNDNDownloader() : IDownloader()
{
  bytesToDownload = 0;
  chunk_number = 0;
}

bool SimpleNDNDownloader::downloadBefore (Ptr<Segment> s, int miliSeconds)
{
  return download(s);
}

uint64_t SimpleNDNDownloader::getPhysicalBitrate()
{
  // Get Device Bitrate
  Ptr<PointToPointNetDevice> nd1 =
      this->m_face->GetNode()->GetDevice(0)->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  nd1->GetAttribute("DataRate", dv);
  DataRate d = dv.Get();
  return d.GetBitRate();
}


bool SimpleNDNDownloader::download (Ptr<Segment> s)
{
  NS_LOG_FUNCTION(this);
  StartApplication ();

  bytesToDownload = s->getSize ();
  cur_chunk_uri = s->getUri();
  chunk_number = 0;

  if(cur_chunk_uri.size () > 7 && cur_chunk_uri.substr (0,7).compare ("http://") == 0)
  {
    cur_chunk_uri = std::string("/").append(cur_chunk_uri.substr (7,cur_chunk_uri.length ()));
  }

  NS_LOG_FUNCTION(cur_chunk_uri << this);

  downloadChunk();
  return true;
}

void SimpleNDNDownloader::downloadChunk ( )
{
  if(bytesToDownload > 0)
  {
    std::stringstream ss;
    ss << cur_chunk_uri << "/chunk_" << chunk_number;

    Ptr<ndn::Name> prefix = Create<ndn::Name> (ss.str ().c_str());

    // Create and configure ndn::Interest
    Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
    UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
    interest->SetNonce (rand.GetValue ());
    interest->SetName (prefix);
    interest->SetInterestLifetime (Seconds (1.0));

    //NS_LOG_FUNCTION("Sending Interest packet for " << *prefix << this);

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest (interest);
    chunk_number++;
  }
}

void SimpleNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  bytesToDownload -= contentObject->GetPayload()->GetSize ();

  if(bytesToDownload > 0)
    downloadChunk ();
  else
  {
    chunk_number = 0;
    NS_LOG_FUNCTION(std::string("Finally received segment: ").append(cur_chunk_uri.substr (0,cur_chunk_uri.find_last_of ("/chunk_"))) << this);
    notifyAll (Observer::No_Message); //notify observers
  }
}

DownloaderType SimpleNDNDownloader::getDownloaderType ()
{
  return SimpleNDN;
}

// Processing upon start of the application
void SimpleNDNDownloader::StartApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StartApplication ();
}

// Processing when application is stopped
void SimpleNDNDownloader::StopApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StopApplication ();
}

void SimpleNDNDownloader::setNodeForNDN (Ptr<Node> node)
{
  NS_LOG_FUNCTION(this);
  SetNode(node);
}

void SimpleNDNDownloader::abortDownload ()
{
  //do something;
}
