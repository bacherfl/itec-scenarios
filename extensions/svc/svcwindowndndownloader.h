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
      SVCWindowNDNDownloader(std::string& cwnd_type);

      // (overriden from WindowNDNDownloader) Call that will be called when a NACK arrives
      virtual void OnNack (Ptr<const ndn::Interest> interest);

      virtual bool download(Ptr<Segment> s);

      virtual bool downloadBefore(Ptr<Segment> s, int miliSeconds);
      virtual void downloadChunk(int chunk_number);

      virtual DownloaderType getDownloaderType ();

    protected:
      //virtual void OnDownloadExpired();

     // EventId needDownloadBeforeEvent;

      //Ptr<ns3::utils::Segment> curSegment;

      //uint64_t deadline;
    };

  }
}

#endif // SVCWINDOWNDNDOWNLOADER_H
