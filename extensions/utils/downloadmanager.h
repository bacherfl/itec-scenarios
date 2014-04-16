#ifndef DOWNLOADMANANGER_H
#define DOWNLOADMANANGER_H

#include "idownloader.h"

#include "simplendndownloader.h"
#include "windowndndownloader.h"

#include "observable.h"
#include "observer.h"

#include <vector>
#include "segment.h"

#include "ns3/node.h"

#include <stdio.h>

#define DOWNLOADER_NUMBER 2

namespace ns3
{
  namespace utils
  {
    class DownloadManager : public Observer, public Observable
    {

    public:

      DownloadManager(DownloaderType dwType, Ptr<ns3::Node> node);

      ~DownloadManager(){}

      void update(ObserverMessage msg);

      void enque(std::vector<Segment*> segments);

      std::vector<Segment*> retriveFinishedSegments();
      std::vector<Segment*> retriveUnfinishedSegments();

  private:


      void segmentReceived();

      std::vector<IDownloader*> downloaders;

      std::vector<Segment*> enquedSegments;
      std::vector<Segment*> finishedSegments;

      void addToFinished(Segment* seg);

      std::vector<IDownloader*> getAllNonBussyDownloaders();
      IDownloader* getFreeDownloader();
      void downloadSegments();

      /*!
       * \brief Resolves the Downloader
       * \param downloader The type of the downloader that is used to download segments.
       * \param node Pointer to ns3 Node which will be used by the Downloader.
       * \return Downloader
       */
      utils::IDownloader* resolveDownloader(utils::DownloaderType downloader, Ptr<Node> node);

    };
  }
}

#endif // DOWNLOADMANANGER_H
