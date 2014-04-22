#ifndef IDOWNLOADER_H
#define IDOWNLOADER_H

#include "segment.h"

#include "../utils/observable.h"
#include "../utils/observer.h"
#include "congestionwindow.h"

#define MAX_PACKET_PAYLOAD 4096
#define PACKET_OVERHEAD    230

#include "ns3-dev/ns3/node.h"
#include "ns3-dev/ns3/ptr.h"

namespace ns3
{
  namespace utils
  {
    enum DownloaderType
    {
      SimpleNDN,
      WindowNDN,
      SVCWindowNDN
    };

    class IDownloader : public utils::Observable
    {
    public:
      IDownloader();

      virtual bool wasSuccessfull();
      virtual bool downloadFinished();
      virtual bool isBussy();
      virtual void reset();

      virtual bool download(Segment *s) = 0;
      virtual bool downloadBefore(Segment *s, int miliSeconds) = 0;
      virtual void abortDownload() = 0;
      virtual const CongestionWindow getCongWindow() = 0;
      virtual void  setCongWindow(const CongestionWindow window) = 0;

      virtual void setNodeForNDN(Ptr <ns3::Node> node) = 0;

      virtual DownloaderType getDownloaderType() = 0;

      virtual Segment* getSegment(){return segment;}

    protected:
      Segment* segment;
      bool lastDownloadSuccessful;
      bool bussy;
      bool finished;
    };
  }
}
#endif // IDOWNLOADER_H
