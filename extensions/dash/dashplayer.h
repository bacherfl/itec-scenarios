#ifndef DASHPLAYER_H
#define DASHPLAYER_H

#include "iadaptationlogic.h"
#include "../utils/observer.h"
#include "../utils/buffer.h"
#include "segment.h"

#include "libdash/libdash.h"

namespace ns3
{
  namespace dashimpl
  {
    //abstract class
    class DashPlayer : utils::Observer
    {
    public:

      DashPlayer(dash::mpd::IMPD *mpd, dashimpl::IAdaptationLogic* alogic, ns3::utils::Buffer* buf);
      virtual ~DashPlayer(){}

      virtual void play();
      virtual void stop();

      virtual void update();

    private:
      dash::mpd::IMPD* mpd;
      dashimpl::IAdaptationLogic* alogic;
      ns3::utils::Buffer* buf;

      bool isPlaying;

    };
  }
}
#endif // DASHPLAYER_H
