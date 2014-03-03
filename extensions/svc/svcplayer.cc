#include "svcplayer.h"

using namespace ns3::svc;

NS_LOG_COMPONENT_DEFINE ("SvcPlayer");

SvcPlayer::SvcPlayer(dash::mpd::IMPD *mpd, std::string dataset_path, utils::IDownloader *downloader,
                     utils::Buffer *buf, unsigned int maxWidth, unsigned int maxHeight)
{
  this->mpd = mpd;
  this->buf = buf;

  this->downloader = downloader;
  this->downloader->addObserver (this);

  this->extractor = new SVCSegmentExtractor(mpd, dataset_path, maxWidth, maxHeight);

  this->isPlaying = false;
  this->isStreaming = false;
}

void SvcPlayer::play()
{

  NS_LOG_FUNCTION(this);
  isPlaying = true;
  streaming ();
  Simulator::Schedule(Seconds(2.0), &SvcPlayer::consume, this);

  /*std::vector<utils::Segment*> segments = extractor->getNextSegments ();
  for(int i = 0; i < segments.size (); i++)
  {
    utils::Segment* s = segments.at (i);
    fprintf(stderr, "SegmentUri %s SegmentLevel %d\n", s->getUri ().c_str (), s->getLevel ());
  }*/
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
      return;
    }

    utils::Segment* cur_seg = current_segments.at(0);

   //w8 if buffer is full
    if(buf->bufferedSeconds () >= (buf->maxBufferSeconds () - cur_seg->getDuration ()))
    {
      Simulator::Schedule(MilliSeconds (100), &SvcPlayer::streaming, this);
      return;
    }

    //check if connection is open or buffer is full...
    while(isStreaming || (buf->bufferedSeconds () >= (buf->maxBufferSeconds () - cur_seg->getDuration ())))
    {
      Simulator::Schedule(MilliSeconds (1), &SvcPlayer::streaming, this);
      return;
    }

    fprintf(stderr, "SvcPlayer::requesting Segment: %s\n", cur_seg->toString ().c_str ());
    downloader->download (cur_seg);
    isStreaming = true;
  }
}

void SvcPlayer::update ()
{
  fprintf(stderr, "NOTIFYED\n");
  //alogic->updateStatistic (dlStartTime, Simulator::Now (), cur_seg->getSize ());

  if(current_segments.at(0)->getLevel() == 0) // only baselayer increases buffer fillstate
  {
    if(!buf->addData(current_segments.at(0)->getDuration()))
    {
      fprintf(stderr, "BUFFER FULL!!!\n");
    }
       //else bufferstatus updated.
  }

  isStreaming = false;
  current_segments.erase (current_segments.begin ());
  streaming ();
}

void SvcPlayer::stop ()
{
  NS_LOG_FUNCTION(this);
  isPlaying = false;
}

void SvcPlayer::consume ()
{
  if(!buf->consumeData (CONSUME_INTERVALL) && isPlaying)
    fprintf(stderr, "CONSUMED FAILED\n");

  Simulator::Schedule(Seconds (CONSUME_INTERVALL), &SvcPlayer::consume, this);
}

