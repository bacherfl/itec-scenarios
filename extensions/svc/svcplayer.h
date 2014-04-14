#ifndef SVCPLAYER_H
#define SVCPLAYER_H

#include "../utils/idownloader.h"
#include "../utils/observer.h"
#include "../utils/buffer.h"
#include "../utils/segment.h"

#include "ns3-dev/ns3/simulator.h"

#include "svcsegmentextractor.h"

#define CONSUME_INTERVALL 1.0
#define REDUCED_BANDWITH 0.75 // estimating we always have max. 75% of the full available bandwidth

namespace ns3
{
    namespace svc
    {
    class SvcPlayer : utils::Observer
    {
    public:
      SvcPlayer(dash::mpd::IMPD* mpd, std::string dataset_path, ns3::utils::IDownloader* downloader,
                ns3::utils::Buffer* buf, unsigned int maxWidth, unsigned int maxHeight);
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

      virtual void update(ObserverMessage msg);

      void streaming();
      void consume();
    };
  }
}


#endif // SVCPLAYER_H
