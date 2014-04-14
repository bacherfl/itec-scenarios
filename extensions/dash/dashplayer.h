#ifndef DASHPLAYER_H
#define DASHPLAYER_H

#include "iadaptationlogic.h"
#include "../utils/idownloader.h"
#include "../utils/observer.h"
#include "../utils/buffer.h"
#include "../utils/segment.h"
#include "../utils/playerlevelhistory.h"

#include "libdash/libdash.h"

#include <unistd.h>
#include "ns3-dev/ns3/simulator.h"

#include <vector>

#define CONSUME_INTERVALL 1.0

namespace ns3
{
  namespace dashimpl
  {

    /*!
     * \brief Implements a DASH PLayer implements Observer pattern and uses PlayerLevelHistory
     */
    class DashPlayer : utils::Observer, PlayerLevelHistory
    {
    public:

      /*!
       * \brief DashPlayer
       * \param mpd The MPD.
       * \param alogic The AdaptationLogic
       * \param buf The buffer for the video data.
       * \param downloader The Downloader that is used to retrieve Segments.
       */
      DashPlayer(dash::mpd::IMPD *mpd, dashimpl::IAdaptationLogic* alogic, ns3::utils::Buffer* buf, std::vector<utils::IDownloader*> downloaders,
                 std::string nodeName);

      /*!
       * \brief Deconstructor ~DashPlayer
       */
      virtual ~DashPlayer(){}

      /*!
       * \brief Starts the playback.
       */
      virtual void play();

      /*!
       * \brief Stops the Playback
       */
      virtual void stop();

      /*!
       * \brief Update method for Observer pattern.
       */
      virtual void update(ObserverMessage msg);

      /*!
       * \brief Consume simulates the playback of video.
       */
      virtual void consume();

    private:

      /*!
       * \brief Starts the streaming of a Segment.
       */
      virtual void streaming();

      /*!
       * \brief The MPD.
       */
      dash::mpd::IMPD* mpd;

      /*!
       * \brief The AdaptationLogic
       */
      dashimpl::IAdaptationLogic* alogic;

      /*!
       * \brief The Downloader
       */
      std::vector<utils::IDownloader*> downloaders;

      /*!
       * \brief The video buffer.
       */
      ns3::utils::Buffer* buf;

      /*!
       * \brief Flag if player is playing the video.
       */
      bool isPlaying;

      /*!
       * \brief Flag if player is streaming some data.
       */
      bool isStreaming;

      /*!
        \brief Name of the node the dash player is on
        */
      std::string m_nodeName;

      /*!
       * \brief List of segments that are currently in the download queue.
       */
      std::vector<utils::Segment*> current_segments;

      /*!
       * \brief Flag that signals if all Segments of the downloaded queue are downloaded.
       */
      bool allSegmentsDownloaded;

      /*!
       * \brief Start time of a particular download.
       */
      Time dlStartTime;

      int downloaderChooser;

    };
  }
}
#endif // DASHPLAYER_H
