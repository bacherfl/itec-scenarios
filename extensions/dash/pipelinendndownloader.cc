#include "pipelinendndownloader.h"

using namespace ns3::dashimpl;

NS_LOG_COMPONENT_DEFINE ("PipelineNDNDownloader");

PipelineNDNDownloader::PipelineNDNDownloader() : IDownloader()
{
  bytesToDownload = 0;
  chunk_number = 0;
}

bool PipelineNDNDownloader::download (Segment *s)
{
  NS_LOG_FUNCTION(this);
  StartApplication ();

  bytesToDownload = s->getSize ();

  unsigned int max_chunks = (int) ceil ( (float)bytesToDownload / (float)MAX_PACKET_PAYLOAD );

  cur_chunk_uri = s->getUri();
  chunk_number = 0;

  if(cur_chunk_uri.size () > 7 && cur_chunk_uri.substr (0,7).compare ("http://") == 0)
  {
    cur_chunk_uri = std::string("/").append(cur_chunk_uri.substr (7,cur_chunk_uri.length ()));
  }


  // create new DownloadStatus and push it into queue
  DownloadStatus* d = new DownloadStatus();
  d->base_uri = cur_chunk_uri.c_str();
  d->bytesToDownload = s->getSize ();
  d->chunks = (int) ceil ( (float)bytesToDownload / (float)MAX_PACKET_PAYLOAD );
  d->chunk_download_status = new bool[d->chunks]();



  if (this->chunks_status.size() != 0)
  {
    // get the last chunk that wasn't downloaded properly
    DownloadStatus* d2 = this->chunks_status.front();

    NS_LOG_FUNCTION(d2->base_uri.c_str () << this);

    for (unsigned int i = 0; i < d2->chunks; i++)
    {
      downloadChunk(d2, i);
    }
  }

  this->chunks_status.push(d);

  // also add the new one to the download queue
  NS_LOG_FUNCTION(d->base_uri.c_str () << this);

  for (unsigned int i = 0; i < d->chunks; i++)
  {
    downloadChunk(d, i);
  }


  Simulator::Schedule(Seconds (0.5), &PipelineNDNDownloader::checkForLostPackets, this);

  return true;
}


void PipelineNDNDownloader::checkForLostPackets()
{
  NS_LOG_FUNCTION(this);
  // check if there is something to download
  if (this->chunks_status.size() != 0)
  {
    DownloadStatus* d = this->chunks_status.front();

    // check if packet was unsuccessful
    bool ok = true;

    unsigned int i = 0;
    while (ok && i < d->chunks)
    {
      ok = ok && d->chunk_download_status[i];
      i++;
    }

    // re-request packets that have not been delivered properly yet
    if (!ok)
    {
      i = 0;
      while (i < d->chunks)
      {
        downloadChunk(d, i);
        i++;
      }

      // schedule another check
      Simulator::Schedule(Seconds (0.5), &PipelineNDNDownloader::checkForLostPackets, this);
    }
  }

}

void PipelineNDNDownloader::downloadChunk (DownloadStatus* d, unsigned int chunk_number)
{
  if(d->bytesToDownload > 0 && d->chunk_download_status[chunk_number] == false)
  {
    std::stringstream ss;
    ss << d->base_uri << "/chunk_" << chunk_number;

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
  }
}

void PipelineNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  // TODO: Find out what chunk this is
  std::string s =  contentObject->GetName().toUri().c_str();
  std::string segment_uri;

  unsigned int pos1 = s.find_last_of("chunk_");
  segment_uri = s.substr(0, pos1-6);

  s = s.substr(pos1+1);
  unsigned int cur_chunk_number = atoi(s.c_str());

  //fprintf(stderr, "DEBUG: %s\n", segment_uri.c_str());
  //fprintf(stderr, "received: %s (chunk %d, size %d)\n", contentObject->GetName().toUri().c_str(), cur_chunk_number, contentObject->GetPayload()->GetSize ());

  std::ostringstream os;

  os << "Received " << contentObject->GetName().toUri().c_str() <<
      " (Chunk: " << cur_chunk_number << ", Size: " <<  contentObject->GetPayload()->GetSize ();


  NS_LOG_FUNCTION(os.str() << this);


  DownloadStatus *d = this->chunks_status.front();

  //fprintf(stderr, "Expecting segment URI: %s\n", d->base_uri.c_str());
  if (d->base_uri == segment_uri)
  {
    d->chunk_download_status[cur_chunk_number] = true;

    // check whether all chunks for this segment have been downloaded
    bool ok = true;

    unsigned int i = 0;
    while (ok && i < d->chunks)
    {
      ok = ok && d->chunk_download_status[i];
      i++;
    }

    if (ok)
    {
      fprintf(stderr, "DEBUG: notifying observer\n");
      NS_LOG_FUNCTION(std::string("Finally received segment: ").append(d->base_uri) << this);

      this->chunks_status.pop();
      delete d;
      notifyAll (); //notify observers
    }
  } else {
    fprintf(stderr, "DEBUG: Segment URI does not match!\n");
  }


  /*bytesToDownload -= contentObject->GetPayload()->GetSize ();
  this->chunkDownloadStatus[cur_chunk_number] = true;

  if(bytesToDownload <= 0)
  {
   // check chunkDownloadStatus (this shouldnt be wrong)
      for (int i = 0; i < max_chunks; i++)
      {
          if (this->chunkDownloadStatus[i] == false)
          {
              fprintf(stderr, "Missing chunk %d --> TODO: request chunk again\n", i);
              // TODO: Request again
          }
      }
  }*/

}

// Processing upon start of the application
void PipelineNDNDownloader::StartApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StartApplication ();
}

// Processing when application is stopped
void PipelineNDNDownloader::StopApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StopApplication ();
}

void PipelineNDNDownloader::setNodeForNDN (Ptr<Node> node)
{
  NS_LOG_FUNCTION(this);
  SetNode(node);
}
