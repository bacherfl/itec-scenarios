#ifndef SVCPLAYER_H
#define SVCPLAYER_H

#include "../utils/idownloader.h"
#include "../utils/observer.h"
#include "../utils/buffer.h"
#include "../utils/segment.h"

#include "svcsegmentextractor.h"

namespace ns3
{
    namespace svc
    {
    class SvcPlayer
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
    };
  }
}


#endif // SVCPLAYER_H
