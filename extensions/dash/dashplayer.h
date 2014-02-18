#ifndef DASHPLAYER_H
#define DASHPLAYER_H

#include "iadaptationlogic.h"
#include "idownloader.h"
#include "../utils/observer.h"
#include "../utils/buffer.h"
#include "segment.h"

#include "libdash/libdash.h"

#include <unistd.h>
#include "ns3-dev/ns3/simulator.h"

#define CONSUME_INTERVALL 1.0

namespace ns3
{
  namespace dashimpl
  {
    //abstract class
    class DashPlayer : utils::Observer
    {
    public:

      DashPlayer(dash::mpd::IMPD *mpd, dashimpl::IAdaptationLogic* alogic, ns3::utils::Buffer* buf, dashimpl::IDownloader* downloader);
      virtual ~DashPlayer(){}

      virtual void play();
      virtual void stop();

      virtual void update();
      virtual void consume();

    private:

      virtual void streaming();

      dash::mpd::IMPD* mpd;
      dashimpl::IAdaptationLogic* alogic;
      dashimpl::IDownloader* downloader;
      ns3::utils::Buffer* buf;

      bool isPlaying;
      bool isStreaming;
      Segment* cur_seg;

      Time dlStartTime;

    };
  }
}
#endif // DASHPLAYER_H
