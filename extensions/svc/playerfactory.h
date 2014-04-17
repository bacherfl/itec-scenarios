#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include "svcplayer.h"

#include "libdash/libdash.h"

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "../utils/idownloader.h"
#include "../utils/buffer.h"
#include "../utils/downloadmanager.h"

namespace ns3
{
  namespace svc
  {
    class PlayerFactory
    {
    public:
      SvcPlayer* createPlayer(std::string mpd_path, unsigned int buffer_size, utils::DownloaderType dwnType,
                              unsigned int maxWidth, unsigned int maxHeight, Ptr<ns3::Node> node);
      static PlayerFactory* getInstance();

    private:
      PlayerFactory();

      dash::mpd::IMPD* resolveMPD(std::string mpd_path) ;

      static PlayerFactory* instance;
      dash::IDASHManager* manager;

      std::string getPWD();
    };
  }
}
#endif // PLAYERFACTORY_H
