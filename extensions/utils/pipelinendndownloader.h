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
#include "ns3-dev/ns3/ndnSIM/utils/ndn-rtt-estimator.h"
#include "ns3-dev/ns3/ndnSIM/utils/ndn-rtt-mean-deviation.h"


#include <ns3-dev/ns3/ndn-l3-protocol.h>

#include <stdio.h>

#include <sstream>
#include <queue>


#define NDN_PIPELINE_SENDPACKET_SCHEDULE 0.25
#define NDN_PIPELINE_SENDPACKET_TIMEOUT 500


namespace ns3
{
  namespace utils
  {
    /**
     * @brief The PipelineNDNDownloader class is DEPRECATED
     */
    class PipelineNDNDownloader : public IDownloader, public ndn::App
    {
    public:

      enum DownloadStatus { NotInitiated= 0, Initiated= 1, Received= 2, Timeout=3  };

      class SegmentStatus
      {
      public:
        std::string base_uri;
        unsigned int chunks;
        DownloadStatus* chunk_download_status;
        Time* chunk_download_time;
        unsigned int bytesToDownload;
      };

      PipelineNDNDownloader();

      virtual bool download (Segment *s);

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void OnData (Ptr<const ndn::Data> contentObject);

      virtual void OnTimeout (uint32_t sequenceNumber);

      virtual void OnNack (Ptr<const ndn::Interest> interest);

      // (overridden from ndn::App) Processing upon start of the application
      virtual void StartApplication ();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void StopApplication ();

      virtual void setNodeForNDN (Ptr<Node> node);

      virtual void checkForSendPackets();


      unsigned int bytesToDownload;
      unsigned int chunk_number;
      std::string cur_chunk_uri;

    protected:
      virtual void downloadChunk(SegmentStatus* d, unsigned int chunk_number);


      unsigned int max_packets;

      unsigned int max_packets_threshold;


      Ptr<ndn::RttEstimator> m_rtt;

      // contains status of all chunks that are not fully queried yet
      std::queue<SegmentStatus*> chunks_status;

    };
  }
}


#endif // PIPELINENDNDOWNLOADER_H
