#include "dashplayer.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;

DashPlayer::DashPlayer(dash::mpd::IMPD* mpd, IAdaptationLogic *alogic, ns3::utils::Buffer* buf)
{
  this->mpd = mpd;
  this->alogic = alogic;
  this->buf = buf;

  isPlaying = false;
}

void DashPlayer::play ()
{
  isPlaying = true;
  Segment* s;

  while(isPlaying)
  {
    s = alogic->getNextSegmentUri();

    fprintf(stderr, "Segment(%s, %d)\n", s->getUri ().c_str (),s->getSize ());

    if(s == NULL)
    {
      isPlaying = false;
      break;
    }
  }
}

void DashPlayer::stop ()
{
  isPlaying = false;
}

void DashPlayer::update ()
{

}

