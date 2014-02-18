#ifndef PIPELINENDNDOWNLOADER_H
#define PIPELINENDNDOWNLOADER_H

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

#include "ns3-dev/ns3/simulator.h"


#include <ns3-dev/ns3/ndn-l3-protocol.h>

#include <stdio.h>

#include <sstream>
#include <queue>





namespace ns3
{
  namespace dashimpl
  {
    class PipelineNDNDownloader : public IDownloader, public ndn::App
    {
    public:
      class DownloadStatus
      {
      public:
        std::string base_uri;
        unsigned int chunks;
        bool* chunk_download_status;
        unsigned int bytesToDownload;
      };

      PipelineNDNDownloader();

      virtual bool download (Segment *s);

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void OnData (Ptr<const ndn::Data> contentObject);

      // (overridden from ndn::App) Processing upon start of the application
      virtual void StartApplication ();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void StopApplication ();

      virtual void setNodeForNDN (Ptr<Node> node);

      // checks for lost packets and re-initiates the request
      virtual void checkForLostPackets();


      unsigned int bytesToDownload;
      unsigned int chunk_number;
      std::string cur_chunk_uri;

    protected:
      virtual void downloadChunk(DownloadStatus* d, unsigned int chunk_number);

      // contains status of all chunks that are not fully queried yet
      std::queue<DownloadStatus*> chunks_status;

    };
  }
}


#endif // PIPELINENDNDOWNLOADER_H
