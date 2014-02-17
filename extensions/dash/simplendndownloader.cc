#include "simplendndownloader.h"

using namespace ns3::dashimpl;

NS_OBJECT_ENSURE_REGISTERED (SimpleNDNDownloader);

SimpleNDNDownloader::SimpleNDNDownloader() : IDownloader()
{
  bytesToDownload = 0;
  chunk_number = 0;
}

bool SimpleNDNDownloader::download (Segment *s)
{
  StartApplication ();

  bytesToDownload = s->getSize ();
  cur_chunk_uri = s->getUri();
  chunk_number = 0;

  if(cur_chunk_uri.size () > 7 && cur_chunk_uri.substr (0,7).compare ("http://") == 0)
  {
    cur_chunk_uri = std::string("/").append(cur_chunk_uri.substr (7,cur_chunk_uri.length ()));
  }

  fprintf(stderr, "Download %s\n", cur_chunk_uri.c_str ());
  downloadChunk();

  return true;
}

void SimpleNDNDownloader::downloadChunk ( )
{
  if(bytesToDownload > 0)
  {

    std::stringstream ss;
    ss << cur_chunk_uri << "/chunk_" << chunk_number;

    fprintf(stderr, "downloadChunk: %s\n", ss.str ().c_str());

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

  fprintf(stderr, "downloadChunk\n");
}

void SimpleNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  fprintf(stderr, "received: %s size %d", contentObject->GetName().toUri().c_str(), contentObject->GetPayload()->GetSize ());

  bytesToDownload -= contentObject->GetPayload()->GetSize ();

  if(bytesToDownload > 0)
    downloadChunk ();

}

// Processing upon start of the application
void SimpleNDNDownloader::StartApplication ()
{
  ndn::App::StartApplication ();
}

// Processing when application is stopped
void SimpleNDNDownloader::StopApplication ()
{
  ndn::App::StopApplication ();
}

void SimpleNDNDownloader::setNodeForNDN (Ptr<Node> node)
{
  SetNode(node);
}
