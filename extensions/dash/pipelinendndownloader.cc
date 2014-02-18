#include "pipelinendndownloader.h"

using namespace ns3::dashimpl;

NS_LOG_COMPONENT_DEFINE ("PipelineNDNDownloader");

PipelineNDNDownloader::PipelineNDNDownloader() : IDownloader()
{
  bytesToDownload = 0;
  chunk_number = 0;
  max_packets = 2;
  max_packets_threshold = 10;

  m_rtt = CreateObject<ndn::RttMeanDeviation> ();
}



bool PipelineNDNDownloader::download (Segment *s)
{
  NS_LOG_FUNCTION(this);
  StartApplication ();
  fprintf(stderr, "Download called\n");

  bytesToDownload = s->getSize ();


  cur_chunk_uri = s->getUri();
  chunk_number = 0;

  if(cur_chunk_uri.size () > 7 && cur_chunk_uri.substr (0,7).compare ("http://") == 0)
  {
    cur_chunk_uri = std::string("/").append(cur_chunk_uri.substr (7,cur_chunk_uri.length ()));
  }

  // create new DownloadStatus and push it into queue
  SegmentStatus* d = new SegmentStatus();
  d->base_uri = cur_chunk_uri.c_str();
  d->bytesToDownload = s->getSize ();
  d->chunks = (int) ceil ( (float)this->bytesToDownload / (float)MAX_PACKET_PAYLOAD );
  d->chunk_download_status = new DownloadStatus[d->chunks];
  d->chunk_download_time   = new Time[d->chunks];

  for (int i = 0; i < d->chunks; i++)
  {
    d->chunk_download_status[i] = NotInitiated;
  }

  this->chunks_status.push(d);

  this->checkForSendPackets();

  return true;
}


// check if there is packets to send
void PipelineNDNDownloader::checkForSendPackets()
{

  if (this->chunks_status.size() > 0)
  {
    bool packet_loss = false;
    bool packet_send = false;
    //fprintf(stderr, "Cong Wind: %d, Thresh: %d\n", max_packets, max_packets_threshold);
    SegmentStatus* d = this->chunks_status.front();
    unsigned int cnt = 0;
    for (unsigned int i = 0; i < d->chunks; i++)
    {
      // check if there is a chunk that has not yet been send { NotInitiated= 0, Initiated= 1, Received= 2, Timeout=3
      if (d->chunk_download_status[i] == NotInitiated || d->chunk_download_status[i] == Timeout)
      {
        // check if we still have free download cont.
        if ( cnt < ceil((double)max_packets/10.0) && cnt < 20 )
        {
          packet_send = true;
          downloadChunk(d, i);
          cnt++;
        }
      } else if (d->chunk_download_status[i] == Initiated)
      {
        // check for timeout
        Time diff(Simulator::Now());
        diff -= d->chunk_download_time[i];

        if (diff.GetMilliSeconds() > NDN_PIPELINE_SENDPACKET_TIMEOUT)
        {
          //fprintf(stderr, "Timeout occured for packet %d - needs to be transmitted again\n", i);
          d->chunk_download_status[i] = Timeout;
          packet_loss = true;
        }
      }
    }

    if (packet_loss)
    {
      max_packets = max_packets / 2;
      if (max_packets < max_packets_threshold)
        max_packets = max_packets_threshold;

      max_packets_threshold = max_packets_threshold / 2;
      if (max_packets_threshold < 2)
        max_packets_threshold = 2;
    } else if (packet_send) {
      if (max_packets >= max_packets_threshold)
      {
        // congestion avoidance
        max_packets++;
      } else {
        // slow start
        max_packets = max_packets * 2;
      }

    }
   // fprintf(stderr, "%d packets send...\n", cnt);
    // no need to schedule next event if there is no data in here
    Simulator::Schedule(Seconds (NDN_PIPELINE_SENDPACKET_SCHEDULE), &PipelineNDNDownloader::checkForSendPackets, this);
  } else {
    fprintf(stderr, "checkForSend: no data available\n");
  }
}



void PipelineNDNDownloader::downloadChunk (SegmentStatus* d, unsigned int chunk_number)
{
  if(d->bytesToDownload > 0 &&
     (d->chunk_download_status[chunk_number] == NotInitiated || d->chunk_download_status[chunk_number] == Timeout)
     )
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

    d->chunk_download_status[chunk_number] = Initiated;
    d->chunk_download_time[chunk_number] = Simulator::Now();

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest (interest);

  }
}



void PipelineNDNDownloader::OnTimeout (uint32_t sequenceNumber)
{
}

void PipelineNDNDownloader::OnNack (Ptr<const ndn::Interest> interest)
{
  App::OnNack(interest);
  fprintf(stderr, "ON Nack called...\n");
}

void PipelineNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{

  // update threshold over time
  //if (max_packets > 5 * max_packets_threshold)
    //max_packets_threshold = max_packets_threshold * 2;


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


  SegmentStatus *d = this->chunks_status.front();

  //fprintf(stderr, "Expecting segment URI: %s\n", d->base_uri.c_str());
  if (d->base_uri == segment_uri)
  {
    d->chunk_download_status[cur_chunk_number] = Received;

    // check whether all chunks for this segment have been downloaded
    bool ok = true;

    unsigned int i = 0;
    while (ok && i < d->chunks)
    {
      ok = ok && (d->chunk_download_status[i] == Received);
      i++;
    }

    if (ok)
    {
      fprintf(stderr, "DEBUG: notifying observer\n");
      NS_LOG_FUNCTION(std::string("Finally received segment: ").append(d->base_uri) << this);

      this->chunks_status.pop();
      delete[] d->chunk_download_status;
      delete[] d->chunk_download_time;
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
