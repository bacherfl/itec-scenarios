#include "svcplayer.h"

using namespace ns3::svc;

NS_LOG_COMPONENT_DEFINE ("SvcPlayer");

SvcPlayer::SvcPlayer(dash::mpd::IMPD *mpd, std::string dataset_path, utils::DownloadManager* dwnManager,
                     utils::Buffer *buf, unsigned int maxWidth, unsigned int maxHeight,
                     std::string nodeName)
{
  this->mpd = mpd;
  this->buf = buf;
  this->m_nodeName = nodeName;
  this->isPlaying = false;

  this->dwnManager = dwnManager;
  this->dwnManager->addObserver (this);

  this->extractor = new SVCSegmentExtractor(mpd, dataset_path, this->buf, maxWidth, maxHeight, dwnManager->getPhysicalBitrate ());
}

void SvcPlayer::play()
{
  NS_LOG_FUNCTION(m_nodeName << this);
  isPlaying = true;
  allSegmentsDownloaded = false;
  this->logDownloadedVideo (mpd->GetBaseUrls ().at (0)->GetUrl());
  streaming ();
  Simulator::Schedule(Seconds(2.0), &SvcPlayer::consume, this);
}

void SvcPlayer::streaming ()
{
  if(isPlaying)
  {
    if(current_segments.size () == 0) // there are still quality levels to download
      current_segments = extractor->getNextSegments ();

    //check if last segment
    if(current_segments.size () == 0)
    {
      isPlaying = false;
      allSegmentsDownloaded = true;
      return;
    }

    utils::Segment* sample = current_segments.at(0);

    //wait if buffer is full
    if(buf->bufferedSeconds () >= (buf->maxBufferSeconds () - sample->getDuration ()))
    {
      Simulator::Schedule(MilliSeconds (100), &SvcPlayer::streaming, this);
      return;
    }

    fprintf(stderr, "Requesting SegmentBunch:\n");
    for(int i = 0; i < current_segments.size (); i++)
    {
      fprintf(stderr, "SVCPlayer::requesting Segment: %s\n", current_segments.at(i)->getUri ().c_str ());
    }

    dlStartTime = Simulator::Now ();
    dwnManager->enque(current_segments);
  }
}

void SvcPlayer::update (ObserverMessage msg)
{

  switch(msg)
  {
    case Observer::SegmentReceived:
    {
      //fprintf(stderr, "SvcPlayer::update SegmentReceived\n");

      addToBuffer(dwnManager->retriveFinishedSegments ());
      current_segments.clear ();
      streaming ();
      break;
    }
    case Observer::NackReceived:
    {
      //todo
      break;
    }
    default:
    {
      //we are not interested in other messages.
    }
  }
}

void SvcPlayer::addToBuffer (std::vector<utils::Segment *> received_segs)
{
  fprintf(stderr, "received_segs.size () = %d\n",(int) received_segs.size ());
  if(received_segs.size () == 0)
  {
    fprintf(stderr, "Cant fetch unfinished data, since even the segment with level 0 is not finished yet..\n");
    return;
  }

  unsigned int total_size = 0;
  utils::Segment* s = NULL;
  utils::Segment* s_highest = received_segs.front ();
  for(int i = 0; i < received_segs.size (); i++)
  {
    s = received_segs.at(i);

    fprintf(stderr, "SVC-Player received for segNumber %u in level %u with size of %u\n",
            s->getSegmentNumber(), s->getLevel(), s->getSize());

    total_size += s->getSize();
    SetPlayerLevel(s->getSegmentNumber(), s->getLevel(), buf->bufferedSeconds(), s->getSize (), (Simulator::Now ().GetMilliSeconds () - dlStartTime.GetMilliSeconds ()));

    if(s->getLevel () > s_highest->getLevel ())
      s_highest = s;
  }

  this->extractor->update(s_highest);

  fprintf(stderr, "SVC-Player received %d segments for segNumber %u with total size of %u\n", (int)received_segs.size (), received_segs.at(0)->getSegmentNumber(), total_size);

  if(!buf->addData (current_segments.front()->getDuration ()))
  {
    NS_LOG_INFO("SVCPlayer(" << m_nodeName << "): BUFFER FULL");
  }
}

void SvcPlayer::stop ()
{
  NS_LOG_FUNCTION(this << m_nodeName);
  this->WriteToFile(m_nodeName + ".txt");
  isPlaying = false;
}

void SvcPlayer::consume ()
{
  if(allSegmentsDownloaded && buf->isEmpty ())
  {
    NS_LOG_INFO("SVCPlayer(" << m_nodeName << "): All Done");
    stop();
    return;
  }

  if(!buf->consumeData (CONSUME_INTERVALL) && isPlaying)
  {
    //ok lets try fetching data from downloadManager
    fprintf(stderr, "Trying to Consume an unfinished BUNCH\n");
    addToBuffer(dwnManager->retriveUnfinishedSegments ());

    if(!buf->consumeData (CONSUME_INTERVALL) && isPlaying)
    {
       NS_LOG_UNCOND("SVCPlayer(" << m_nodeName << "): CONSUME FAILED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
       logUnsmoothSecond (current_segments.at (0)->getSegmentNumber(), CONSUME_INTERVALL);
    }
    else
    {
    //ommit old requests
    current_segments.clear ();
    //start streaming next segment(s)
    streaming();
    }
  }

  Simulator::Schedule(Seconds (CONSUME_INTERVALL), &SvcPlayer::consume, this);
}

