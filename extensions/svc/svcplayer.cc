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

    //wait if buffer is full
    if(buf->bufferedSeconds () >= (buf->maxBufferSeconds () - cur_seg->getDuration ()))
    {
      Simulator::Schedule(MilliSeconds (100), &SvcPlayer::streaming, this);
      return;
    }

    //check if connection is open or buffer is full...
    if(isStreaming || (buf->bufferedSeconds () >= (buf->maxBufferSeconds () - cur_seg->getDuration ())))
    {
      Simulator::Schedule(MilliSeconds (1), &SvcPlayer::streaming, this);
      return;
    }

    //fprintf(stderr, "SvcPlayer::requesting Segment: %s\n", cur_seg->toString ().c_str ());


    // always download segment level 0
    if (cur_seg->getLevel() == 0)
    {
      downloader->download(cur_seg);
    } else
    { // but if higher level, set a threshold time until the download MUST be completed to be useful
      int best_before = buf->bufferedSeconds () * 1000 - 500;
      if (best_before < 2000)
        best_before = 2000;

      // check if it is feasible to download this segment
      if (cur_seg->getAvgLvlBitrate() > downloader->getPhysicalBitrate() * REDUCED_BANDWITH)
      {
        fprintf(stderr, "SvcPlayer::aborting segment %s because of bandwidth\n", cur_seg->toString ().c_str ());

        current_segments.erase (current_segments.begin ());

        // just get the next segment
        Simulator::Schedule(MilliSeconds (1), &SvcPlayer::streaming, this);
        return;
      }

      downloader->downloadBefore (cur_seg, best_before);
    }

    isStreaming = true;
  }
}

void SvcPlayer::update ()
{
  //fprintf(stderr, "NOTIFYED\n");
  //alogic->updateStatistic (dlStartTime, Simulator::Now (), cur_seg->getSize ());

  if(downloader->wasSuccessfull())
  {
    if(current_segments.at(0)->getLevel() == 0) // only baselayer increases buffer fillstate
    {
      if(!buf->addData(current_segments.at(0)->getDuration()))
      {
        fprintf(stderr, "BUFFER FULL!!!\n");
      }
    }
  }
  else
  {
    NS_LOG_INFO("SvcPlayer::update Segment " << (*current_segments.begin ())->toString().c_str() << "was dropped \n");
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

