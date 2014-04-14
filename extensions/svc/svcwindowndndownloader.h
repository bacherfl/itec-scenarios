#ifndef SVCWINDOWNDNDOWNLOADER_H
#define SVCWINDOWNDNDOWNLOADER_H

#include "../utils/windowndndownloader.h"
#include "svcleveltag.h"
#include "../utils/deadlinetag.h"
#include "svcbitratetag.h"


namespace ns3
{
  namespace utils
  {
    class SVCWindowNDNDownloader : public WindowNDNDownloader
    {
    public:
      SVCWindowNDNDownloader();


      // (overridden from WindowNDNDownloader) Callback that will be called when Data arrives
      virtual void OnData (Ptr<const ndn::Data> contentObject);

      // (overriden from WindowNDNDownloader) Call that will be called when a NACK arrives
      virtual void OnNack (Ptr<const ndn::Interest> interest);

      virtual bool download(Segment *s);


      virtual bool downloadBefore(Segment *s, int miliSeconds);
      virtual void downloadChunk(int chunk_number);

      virtual DownloaderType getDownloaderType ();

      void notifyAll(Observer::ObserverMessage msg);

    protected:
      virtual bool isPartOfCurrentSegment(std::string packetUri);

      virtual void OnDownloadExpired();

      EventId needDownloadBeforeEvent;

      uint64_t deadline;
    };

  }
}

#endif // SVCWINDOWNDNDOWNLOADER_H
