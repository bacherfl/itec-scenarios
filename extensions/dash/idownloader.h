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
  namespace dashimpl
  {
    enum DownloaderType
    {
      SimpleNDN,
      PipelineNDN,
      WindowNDN
    };

    class IDownloader : public utils::Observable
    {
    public:
      IDownloader();
      virtual bool download(Segment *s) = 0;
      virtual void setNodeForNDN(Ptr <ns3::Node> node) = 0;
    };
  }
}
#endif // IDOWNLOADER_H
