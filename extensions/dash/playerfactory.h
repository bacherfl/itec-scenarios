#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "dashplayer.h"

#include "libdash/libdash.h"

#include "libdash/IMPD.h"
#include "iadaptationlogic.h"

#include "alwayslowestadaptationlogic.h"

#include <string>
#include "../utils/buffer.h"

#include <stdio.h>

namespace ns3
{
  namespace dashimpl
  {  
    class PlayerFactory
    {
    public:
      DashPlayer* createPlayer(std::string mpd_path, AdaptationLogicType alogic, unsigned int buffer_size);
      static PlayerFactory* getInstance();

    private:
      PlayerFactory();

      IAdaptationLogic* resolveAdaptation(AdaptationLogicType alogic, dash::mpd::IMPD* mpd, std::string dataset_path);
      dash::mpd::IMPD* resolveMPD(std::string mpd_path) ;
      std::string getPWD();

      static PlayerFactory* instance;
      dash::IDASHManager* manager;

    };
  }
}

#endif // PLAYERFACTORY_H
