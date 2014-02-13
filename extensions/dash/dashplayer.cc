#include "dashplayer.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;

DashPlayer::DashPlayer(dash::mpd::IMPD* mpd, IAdaptationLogic *alogic, ns3::utils::Buffer* buf)
{
  this->mpd = mpd;
  this->alogic = alogic;
  this->buf = buf;
}

void DashPlayer::play ()
{
  alogic->getNextSegmentUri();
}

void DashPlayer::stop ()
{

}

void DashPlayer::update ()
{

}

