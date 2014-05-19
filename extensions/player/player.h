#ifndef PLAYER_H
#define PLAYER_H

#include "ns3/simple-ref-count.h"

#include "libdash/libdash.h"

#include <unistd.h>
#include "ns3-dev/ns3/simulator.h"

#include "../utils/downloadmanager.h"
#include "../utils/playerlevelhistory.h"

namespace ns3
{
  namespace player
  {
    class Player : public ns3::SimpleRefCount<Player>, utils::Observer, PlayerLevelHistory
    {
    public:
      Player(dash::mpd::IMPD *mpd, utils::DownloadManager* dwnManager, std::string nodeName);

      void play();
      void stop();

    private:
      utils::DownloadManager* dwnManager;
      dash::mpd::IMPD *mpd;

      bool isPlaying;
      std::string m_nodeName;
    };
  }
}

#endif // PLAYER_H
