#ifndef SIMPLENDNDOWNLOADER_H
#define SIMPLENDNDOWNLOADER_H

#include "idownloader.h"

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/string.h"
#include "ns3-dev/ns3/callback.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/random-variable.h"

#include "ns3-dev/ns3/ndn-app-face.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-data.h"
#include "ns3-dev/ns3/ndn-fib.h"

#include <ns3-dev/ns3/ndn-l3-protocol.h>

#include <stdio.h>

#include <sstream>

namespace ns3
{
  namespace utils
  {
    class SimpleNDNDownloader : public IDownloader, public ndn::App
    {
    public:
      SimpleNDNDownloader();

      virtual bool download (Segment *s);
      virtual bool downloadBefore(Segment *s, int miliSeconds);

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void OnData (Ptr<const ndn::Data> contentObject);

      // (overridden from ndn::App) Processing upon start of the application
      virtual void StartApplication ();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void StopApplication ();

      virtual void setNodeForNDN (Ptr<Node> node);

      unsigned int bytesToDownload;
      unsigned int chunk_number;
      std::string cur_chunk_uri;

    protected:
      virtual void downloadChunk();

    };
  }
}

#endif // SIMPLENDNDOWNLOADER_H
