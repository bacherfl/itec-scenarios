#include "svcplayer.h"

using namespace ns3::svc;

NS_LOG_COMPONENT_DEFINE ("SvcPlayer");

SvcPlayer::SvcPlayer(dash::mpd::IMPD *mpd, std::string dataset_path, utils::DownloadManager* dwnManager,
                     utils::Buffer *buf, unsigned int maxWidth, unsigned int maxHeight,
                     std::string nodeName)
{
  this->mpd = mpd;
  this->buf = buf;

  this->dwnManager = dwnManager;
  this->dwnManager->addObserver (this);

  this->extractor = new SVCSegmentExtractor(mpd, dataset_path, maxWidth, maxHeight);

  this->isPlaying = false;

  this->m_nodeName = nodeName;
}

void SvcPlayer::play()
{
  NS_LOG_FUNCTION(m_nodeName << this);
  isPlaying = true;
  allSegmentsDownloaded = false;
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

      // check which segments are feasible to be download and remove the rest
    for(std::vector<utils::Segment*>::iterator it = current_segments.begin (); it != current_segments.end ();)
    {
      if((*it)->getLevel() != 0 && (*it)->getAvgLvlBitrate() > dwnManager->getPhysicalBitrate() * REDUCED_BANDWITH)
        current_segments.erase (it);
      else
        ++it;
    }

    fprintf(stderr, "Requesting SegmentBunch:\n");
    for(int i = 0; i < current_segments.size (); i++)
    {
      fprintf(stderr, "SVCPlayer::requesting Segment: %s\n", current_segments.at(i)->getUri ().c_str ());
    }

    dwnManager->enque(current_segments);
  }
}

void SvcPlayer::update (ObserverMessage msg)
{

  if(msg == Observer::SegmentReceived)
  {
    fprintf(stderr, "SvcPlayer::update SegmentReceived\n");

    addToBuffer(dwnManager->retriveFinishedSegments ());
    current_segments.clear ();
    streaming ();
  }
  else
    fprintf(stderr, "SvcPlayer::update UNHANDELD MESSAGE");
}

void SvcPlayer::addToBuffer (std::vector<utils::Segment *> received_segs)
{
  fprintf(stderr, "received_segs.size () = %d\n",received_segs.size ());
  if(received_segs.size () == 0)
  {
    fprintf(stderr, "Cant fetch unfinished data, since even the segment with level 0 is not finished yet..\n");
    return;
  }

  std::vector<utils::Segment *> checked_segs;
  //check if segment levels are continous.
  bool foundLayer = false;
  for(int i = 0; i < received_segs.size (); i++)
  {
    for(int j = 0; j < received_segs.size (); j++)
    {
      if(received_segs.at (j)->getLevel() == i)
      {
        checked_segs.push_back (received_segs.at (j));
        foundLayer = true;
        break;
      }
    }
    if(!foundLayer)
      break;
    foundLayer = false;
  }

  unsigned int total_size = 0;
  for(int i = 0; i < checked_segs.size (); i++)
  {
    fprintf(stderr, "SVC-Player received for segNumber %u in level %u with size of %u\n",
            checked_segs.at(i)->getSegmentNumber(), checked_segs.at(i)->getLevel(), checked_segs.at(i)->getSize());


    total_size += checked_segs.at(i)->getSize();
    SetPlayerLevel(checked_segs.at(i)->getSegmentNumber(), checked_segs.at(i)->getLevel(), buf->bufferedSeconds());
  }

  fprintf(stderr, "SVC-Player received %d segments for segNumber %u with total size of %u\n", (int)checked_segs.size (), checked_segs.at(0)->getSegmentNumber(), total_size);

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
      NS_LOG_ERROR("SVCPlayer(" << m_nodeName << "): CONSUME FAILED"); //ok we stall
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

