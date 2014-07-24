#ifndef PLAYER_H
#define PLAYER_H

#include "ns3/simple-ref-count.h"

#include "libdash/libdash.h"

#include <unistd.h>
#include "ns3-dev/ns3/simulator.h"

#include "../utils/downloadmanager.h"
#include "../utils/playerlevelhistory.h"
#include "layeredadaptationlogic.h"
#include "layeredbuffer.h"


#define CONSUME_INTERVALL 1.0
#define CONSUME_DELAY     0.5
#define STARTUP_DELAY     2.0


namespace ns3
{
  namespace player
  {
    class Player : public ns3::SimpleRefCount<Player>, utils::Observer, PlayerLevelHistory
    {
    public:
      Player(dash::mpd::IMPD *mpd, dashimpl::IAdaptationLogic *alogic,
             Ptr<ns3::player::LayeredBuffer> buf,
             utils::DownloadManager* dwnManager, std::string nodeName);

      void play();
      void stop();

    protected:
      virtual void update(ObserverMessage msg);

      /*!
       * \brief Starts the streaming of a Segment.
       */
      void streaming();

      /*!
      * \brief Consume simulates the playback of video.
      */
      void consuming();

      /*!
       * \brief Flag if player is playing the video.
       */
      bool isPlaying;

      /*!
        \brief Name of the node the dash player is on
        */
      std::string m_nodeName;

      /*!
       * \brief List of segments that are currently in the download queue.
       */
      Ptr<utils::Segment> current_segment;

      /*!
       * \brief Indicates whether the player has started to stream.
       */
      bool hasStarted;

      /*!
        * \brief Indicates whether the player has finished streaming.
       */
      bool hasFinished;

      /*!
       * \brief Start time of a particular download.
       */
      Time dlStartTime;

      virtual void SetPlayerLevel (unsigned int segmentNumber, unsigned int level, unsigned int buffer, unsigned int segSize, int64_t dlDuration);

    private:
      utils::DownloadManager* dwnManager;
      dash::mpd::IMPD *mpd;
      Ptr<ns3::player::LayeredBuffer> buf;
      ns3::dashimpl::IAdaptationLogic* alogic;



      EventId streamingEvent;
      EventId consumeEvent;

      void scheduleNextStreaming(double seconds);
      void scheduleNextConsumeEvent(double seconds);

      /*!
        * \brief the next segment number that needs to be played; -1 if nothing has been played yet
       */
      int currentSegmentNumber;
    };
  }
}

#endif // PLAYER_H
