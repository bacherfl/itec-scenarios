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

#include "../utils/idownloader.h"
#include "../utils/buffer.h"
#include "../utils/simplendndownloader.h"
#include "../utils/windowndndownloader.h"

namespace ns3
{
  namespace dashimpl
  {
    /*!
    \class PlayerFactory
    \brief The PlayerFactory is used to instantiate a new DashPlayer. It is implemented as Singleton.
    */
    class PlayerFactory
    {
    public:

      /*!
       \brief Constructs a new DashPlayer.
      \param mpd_path The path to the MPD file.
      \param alogic The type of the adaptationlogic that is used to determine the adaptation decisions.
      \param buffer_size The size of the video buffer in seconds.
      \param downloader The type of the downloader that is used to download segments.
      \param node Pointer to ns3 Node Object on which the DashPlayer will run.
       \return A DashPlayer Instance.
       */
      DashPlayer* createPlayer(std::string mpd_path, AdaptationLogicType alogic, unsigned int buffer_size, utils::DownloaderType downloader, Ptr<ns3::Node> node);

      /*!
       * \brief getInstance
       * \return Singleton instance of the PlayerFactory.
       */
      static PlayerFactory* getInstance();

    private:

      /*!
       * \brief PlayerFactory Constructor is private, since this class implements the Singleton patterns. \see PlayerFactory::getInstance()
       */
      PlayerFactory();

      /*!
       * \brief Resolves the AdaptationLogic
       * \param alogic The type of the adaptationlogic that is used to determine the adaptation decisions.
       * \param mpd_path The path to the MPD file.
       * \param dataset_path The path to the DASH dataset.
       * \param buf The Buffer which stores video data.
       * \return The determined AdaptationLogic.
       */
      IAdaptationLogic* resolveAdaptation(AdaptationLogicType alogic, dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer *buf);

      /*!
       * \brief Resolves the MPD.
       * \param mpd_path The path to the MPD file.
       * \return Pointer to IMPD according to LibDASH
       */
      dash::mpd::IMPD* resolveMPD(std::string mpd_path) ;

      /*!
       * \brief Resolves the Downloader
       * \param downloader The type of the downloader that is used to download segments.
       * \param node Pointer to ns3 Node which will be used by the Downloader.
       * \return Downloader
       */
      utils::IDownloader* resolveDownloader(utils::DownloaderType downloader, Ptr<Node> node);

      /*!
       * \brief Determines the current user's homepath.
       * \return PWD
       */
      std::string getPWD();

      /*!
       * \brief Singleton instance of the PlayerFactory.
       */
      static PlayerFactory* instance;

      /*!
       * \brief LibDASH MPDMananger.
       */
      dash::IDASHManager* manager;

    };
  }
}

#endif // PLAYERFACTORY_H
