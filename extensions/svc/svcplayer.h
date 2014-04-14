#ifndef SVCPLAYER_H
#define SVCPLAYER_H

#include "../utils/idownloader.h"
#include "../utils/observer.h"
#include "../utils/buffer.h"
#include "../utils/segment.h"
#include "../utils/playerlevelhistory.h"

#include "ns3-dev/ns3/simulator.h"

#include "svcsegmentextractor.h"


#define CONSUME_INTERVALL 1.0
#define REDUCED_BANDWITH 0.75 // estimating we always have max. 75% of the full available bandwidth

namespace ns3
{
    namespace svc
    {
    class SvcPlayer : utils::Observer, PlayerLevelHistory
    {
    public:
      SvcPlayer(dash::mpd::IMPD* mpd, std::string dataset_path, ns3::utils::IDownloader* downloader,
                ns3::utils::Buffer* buf, unsigned int maxWidth, unsigned int maxHeight,
                std::string nodeName);
      void play();
      void stop();

    private:
      dash::mpd::IMPD* mpd;
      ns3::utils::IDownloader* downloader;
      ns3::utils::Buffer* buf;
      SVCSegmentExtractor* extractor;

      std::vector<utils::Segment*> current_segments;

      bool isPlaying;
      bool isStreaming;

      /*!
       * \brief Flag that signals if all Segments of the downloaded queue are downloaded.
       */
      bool allSegmentsDownloaded;

      /*!
        \brief Name of the node the dash player is on
        */
      std::string m_nodeName;

      virtual void update(ObserverMessage msg);

      void streaming();
      void consume();
    };
  }
}


#endif // SVCPLAYER_H
