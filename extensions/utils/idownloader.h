#ifndef IDOWNLOADER_H
#define IDOWNLOADER_H

#include "segment.h"

#include "../utils/observable.h"
#include "../utils/observer.h"

#define MAX_PACKET_PAYLOAD 4096

#include "ns3-dev/ns3/node.h"
#include "ns3-dev/ns3/ptr.h"

namespace ns3
{
  namespace utils
  {
    enum DownloaderType
    {
      SimpleNDN,
      PipelineNDN,
      TimeoutNDN,
      WindowNDN,
      SVCWindowNDN
    };

    class IDownloader : public utils::Observable
    {
    public:
      IDownloader();

      bool wasSuccessfull();

      virtual bool download(Segment *s) = 0;
      virtual bool downloadBefore(Segment *s, int miliSeconds) = 0;

      virtual void setNodeForNDN(Ptr <ns3::Node> node) = 0;

    protected:
      bool lastDownloadSuccessful;
    };
  }
}
#endif // IDOWNLOADER_H
