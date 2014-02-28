#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/node.h"
#include "ns3-dev/ns3/ptr.h"

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <string>
#include <stdio.h>

#include "dashplayer.h"

#include "libdash/libdash.h"

#include "libdash/IMPD.h"
#include "iadaptationlogic.h"

#include "alwayslowestadaptationlogic.h"
#include "ratebasedadaptationlogic.h"
#include "bufferbasedadaptationlogic.h"

#include "../utils/idownloader.h"
#include "../utils/buffer.h"
#include "../utils/simplendndownloader.h"
#include "../utils/pipelinendndownloader.h"

namespace ns3
{
  namespace dashimpl
  {
    class PlayerFactory
    {
    public:
      DashPlayer* createPlayer(std::string mpd_path, AdaptationLogicType alogic, unsigned int buffer_size, utils::DownloaderType downloader, Ptr<ns3::Node> node);
      static PlayerFactory* getInstance();

    private:
      PlayerFactory();

      IAdaptationLogic* resolveAdaptation(AdaptationLogicType alogic, dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer *buf);
      dash::mpd::IMPD* resolveMPD(std::string mpd_path) ;
      utils::IDownloader* resolveDownloader(utils::DownloaderType downloader, Ptr<Node> node);
      std::string getPWD();

      static PlayerFactory* instance;
      dash::IDASHManager* manager;

    };
  }
}

#endif // PLAYERFACTORY_H
