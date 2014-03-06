#ifndef SVCWINDOWNDNDOWNLOADER_H
#define SVCWINDOWNDNDOWNLOADER_H

#include "windowndndownloader.h"
#include "svcleveltag.h"


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

    protected:
      virtual bool isPartOfCurrentSegment(std::string packetUri);
    };

  }
}

#endif // SVCWINDOWNDNDOWNLOADER_H
