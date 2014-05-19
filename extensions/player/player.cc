#include "player.h"

using namespace ns3::player;

Player::Player(dash::mpd::IMPD *mpd,utils::DownloadManager* dwnManager, std::string nodeName)
{
  this->mpd = mpd;
  //this->alogic = alogic;
  //this->buf = buf;
  this->dwnManager = dwnManager;

  this->dwnManager->addObserver(this);

  isPlaying = false;
  this->m_nodeName = nodeName;
}

void Player::play (){

}

void Player::stop (){

}
