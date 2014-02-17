#include "dashplayer.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;

DashPlayer::DashPlayer(dash::mpd::IMPD* mpd, IAdaptationLogic *alogic, ns3::utils::Buffer* buf, IDownloader *downloader)
{
  this->mpd = mpd;
  this->alogic = alogic;
  this->buf = buf;
  this->downloader = downloader;

  isPlaying = false;
}

void DashPlayer::play ()
{
  isPlaying = true;
  Segment* s;

  //while(isPlaying)
  //{
    s = alogic->getNextSegmentUri();

    if(s == NULL)
    {
      isPlaying = false;
      //break;
    }

    //Todo download segment
    downloader->download (s);
    //fprintf(stderr, "Segment(%s, %d, %d)\n", s->getUri ().c_str (),s->getSize (), s->getDuration ());
  //}
}

void DashPlayer::stop ()
{
  isPlaying = false;
}

void DashPlayer::update ()
{

}
