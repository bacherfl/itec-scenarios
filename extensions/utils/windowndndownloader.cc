#include "windowndndownloader.h"
#include "ns3-dev/ns3/ndn-wire.h"
#include "svcleveltag.h"

using namespace ns3;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("WindowNDNDownloader");

WindowNDNDownloader::WindowNDNDownloader() : IDownloader()
{
  packets_received = 0;
  packets_timeout = 0;
  packets_inflight = 0;
  packets_nack = 0;

  this->curSegmentStatus.chunk_status = NULL;
  this->curSegmentStatus.chunk_timeout_events = NULL;

  this->cwnd.SetThreshold(33);
  this->cwnd.SetReceiverWindowSize(101);


  m_rtt = CreateObject<ndn::RttMeanDeviation> ();
  m_rtt->SetMinRto(Seconds(0.1));
}

bool WindowNDNDownloader::download (Segment *s)
{
  NS_LOG_FUNCTION(this);
  StartApplication ();

  // Get Device Bitrate
  Ptr<PointToPointNetDevice> nd1 = this->m_face->GetNode()->GetDevice(0)->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  nd1->GetAttribute("DataRate", dv);
  DataRate d = dv.Get();

  uint64_t bitrate = d.GetBitRate();

  int max_packets = bitrate / (MAX_PACKET_PAYLOAD + 20) / 8;
  /*fprintf(stderr, "Bitrate: %d\n", bitrate);
  fprintf(stderr, "Max Packets per second = %d\n", max_packets);*/

  // set threshold to max_packets / 4
  cwnd.SetReceiverWindowSize(max_packets);
  cwnd.SetThreshold(max_packets/4);

  if (this->curSegmentStatus.chunk_status != NULL)
    delete this->curSegmentStatus.chunk_status;

  if (this->curSegmentStatus.chunk_timeout_events != NULL)
    delete[] this->curSegmentStatus.chunk_timeout_events;


  this->curSegmentStatus.base_uri = s->getUri();
  this->curSegmentStatus.bytesToDownload = s->getSize();

  // init number of chunks
  this->curSegmentStatus.num_chunks =
      (int) ceil ( (float)  (this->curSegmentStatus.bytesToDownload) / (float)MAX_PACKET_PAYLOAD );


  // init the internal status arrays
  this->curSegmentStatus.chunk_status = new ns3::NDNDownloadStatus[this->curSegmentStatus.num_chunks]();
  this->curSegmentStatus.chunk_timeout_events = new ns3::EventId[this->curSegmentStatus.num_chunks]();

  // init stats
  packets_received = 0;
  packets_timeout = 0;
  packets_inflight = 0;
  packets_nack = 0;
  lastDownloadSuccessful = true;

  // clear the last rtt information
  m_rtt->Reset();
  m_rtt->ClearSent();

  // reset the congestion window
  //cwnd.Reset();

  if(this->curSegmentStatus.base_uri.size () > 7 &&
     this->curSegmentStatus.base_uri.substr (0,7).compare ("http://") == 0)
  {
    this->curSegmentStatus.base_uri = std::string("/").append(
          this->curSegmentStatus.base_uri.substr (7,this->curSegmentStatus.base_uri.length ()));
  }

  NS_LOG_FUNCTION(this->curSegmentStatus.base_uri << this);

  downloadChunk(0);
  return true;
}


int WindowNDNDownloader::GetNextNeededChunkNumber()
{
  return GetNextNeededChunkNumber(0);
}


int WindowNDNDownloader::GetNextNeededChunkNumber(int start_chunk_number)
{
  if (this->curSegmentStatus.num_chunks > 0 && this->curSegmentStatus.chunk_status != NULL)
  {
    int i = start_chunk_number;
    // find first segment that has not been downloaded yet, but is not currently being downloaded
    while (i < this->curSegmentStatus.num_chunks)
    {
      if (this->curSegmentStatus.chunk_status[i] != Received &&
          this->curSegmentStatus.chunk_status[i] != Requested)
      {
        return i;
      }

      i++;
    }
  }
  return -1;
}


void WindowNDNDownloader::ScheduleNextChunkDownload()
{
  NS_LOG_FUNCTION(this);
  int chunk_number = GetNextNeededChunkNumber();

  if (chunk_number == -1)
  {
    //fprintf(stderr, "in WindowNDNDownloader::ScheduleNextChunkDownload() - no chunks available to download (bytesToDownload = %d)\n", this->curSegmentStatus.bytesToDownload);
    return;
  }

  this->scheduleDownloadTimer.Cancel();

  // calculate when to download the next chunk
  this->scheduleDownloadTimer =
      Simulator::Schedule(MilliSeconds(1000.0 / (float)this->cwnd.GetWindowSize()),
                          &WindowNDNDownloader::downloadChunk, this, chunk_number);

}

Ptr<ndn::Interest> WindowNDNDownloader::prepareInterstForDownload (int chunk_number)
{
  // set chunk status to requested
  this->curSegmentStatus.chunk_status[chunk_number] = Requested;

  // get current RTO
  double rto = m_rtt->RetransmitTimeout().GetSeconds();
  rto = std::min<double>(rto, WINDOW_MAX_RTO); // RTO must not be higher than 1 second here

  std::stringstream ss;
  ss << this->curSegmentStatus.base_uri << "/chunk_" << chunk_number;

  Ptr<ndn::Name> prefix = Create<ndn::Name> (ss.str ().c_str());

  // Create and configure ndn::Interest
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce (rand.GetValue ());
  interest->SetName (prefix);
  interest->SetInterestLifetime (Seconds (rto)); // set interest life time equal to the event we are fireing

  // get the timeout event set up
  this->curSegmentStatus.chunk_timeout_events[chunk_number] =
    Simulator::Schedule(Seconds(rto), &WindowNDNDownloader::CheckRetrieveTimeout, this, chunk_number);

  // tell the RTO estimator that we sent out a packet
  m_rtt->SentSeq (SequenceNumber32 (chunk_number), 1);

  // increase packets_inflight
  packets_inflight++;

  //NS_LOG_FUNCTION("Sending Interest packet for " << *prefix << this);

  if (this->scheduleDownloadTimer.IsExpired())
  {
    ScheduleNextChunkDownload();
  }

  return interest;
}

void WindowNDNDownloader::downloadChunk (int chunk_number)
{
  if(this->curSegmentStatus.bytesToDownload > 0)
  {
    Ptr<ndn::Interest> interest = prepareInterstForDownload(chunk_number);

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest (interest);
  }
}


void WindowNDNDownloader::CheckRetrieveTimeout(int c_chunk_number)
{
  // timed out (else the event would have been canceled)
  OnTimeout(c_chunk_number);
}



void WindowNDNDownloader::OnTimeout (int c_chunk_number)
{
  NS_LOG_FUNCTION("Timeout on chunk " << c_chunk_number << ". " << this);

  fprintf(stderr, "Timeout chunk %d, cwnd = %d, in_flight=%d\n", c_chunk_number, cwnd.GetWindowSize(), this->packets_inflight);

  // adjust stats
  this->packets_inflight--;
  this->packets_timeout++;

  // adjust windows/rto
  m_rtt->IncreaseMultiplier ();
  cwnd.DecreaseWindow();

  // re-request the packet
  this->curSegmentStatus.chunk_status[c_chunk_number] = Timeout;


  // make sure that the next packet is scheduled again
  if (this->scheduleDownloadTimer.IsExpired())
  {
    ScheduleNextChunkDownload();
  }
}


void WindowNDNDownloader::OnNack (Ptr<const ndn::Interest> interest)
{
  // find out what chunk this is
  std::string s =  interest->GetName().toUri().c_str();
  std::string segment_uri;
  unsigned int pos1 = s.find_last_of("chunk_");
  segment_uri = s.substr(0, pos1-6);
  s = s.substr(pos1+1);
  int c_chunk_number = atoi(s.c_str());

  fprintf(stderr, "WindowNDNDownloader::OnNack: received NACK for URI: %s\n", interest->GetName ().toUri().c_str());

  NS_LOG_FUNCTION("NACK on chunk " << c_chunk_number << ". " << this);

  // make sure to cancel the OnTimeout event for this chunk
  this->curSegmentStatus.chunk_timeout_events[c_chunk_number].Cancel();

  // adjust stats
  this->packets_inflight--;
  this->packets_nack++;

  // adjust windows/rto
  m_rtt->IncreaseMultiplier ();
  cwnd.DecreaseWindow();

  // re-request the packet
  this->curSegmentStatus.chunk_status[c_chunk_number] = Timeout;
  this->scheduleDownloadTimer.Cancel();

  // make sure that the next packet is scheduled again
  ScheduleNextChunkDownload();
}

void WindowNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_FUNCTION(this);

  this->curSegmentStatus.bytesToDownload -= contentObject->GetPayload()->GetSize ();
  packets_received++;
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
  // tell the congestion window that we received the packet as we expected to receive it
  cwnd.IncreaseWindow();

  // Set the chunk status to received
  this->curSegmentStatus.chunk_status[c_chunk_number] = Received;

  // make sure to cancel the OnTimeout event for this chunk
  this->curSegmentStatus.chunk_timeout_events[c_chunk_number].Cancel();

  if (this->curSegmentStatus.bytesToDownload < 0)
  {
    fprintf(stderr, "< 0...\n");
  }

  if(this->curSegmentStatus.bytesToDownload == 0)
  {
    // cancel the scheduled download timer
    this->scheduleDownloadTimer.Cancel();

    NS_LOG_FUNCTION(std::string("Finally received segment: ").append(curSegmentStatus.base_uri.substr (0,curSegmentStatus.base_uri.find_last_of ("/chunk_"))) << this);
    notifyAll (); //notify observers
  }
  else
  {
    // cancel the next download timer, we have a bigger congestion window now, which means less waiting time
    // make sure that the next packet is scheduled again
      ScheduleNextChunkDownload();
  }
}



void WindowNDNDownloader::CancelAllTimeoutEvents()
{
  for (int i = 0; i < this->curSegmentStatus.num_chunks; i++)
  {
    if (this->curSegmentStatus.chunk_status[i] == Requested)
    {
      this->curSegmentStatus.chunk_timeout_events[i].Cancel();
    }
  }
}

// Processing upon start of the application
void WindowNDNDownloader::StartApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StartApplication ();
}

// Processing when application is stopped
void WindowNDNDownloader::StopApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StopApplication ();
}

void WindowNDNDownloader::setNodeForNDN (Ptr<Node> node)
{
  NS_LOG_FUNCTION(this);
  SetNode(node);
}
