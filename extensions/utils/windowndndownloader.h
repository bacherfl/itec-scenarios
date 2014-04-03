#ifndef WINDOWNDNDOWNLOADER_H
#define WINDOWNDNDOWNLOADER_H

#include "idownloader.h"

#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"


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
#include "ns3-dev/ns3/ndnSIM/utils/ndn-rtt-estimator.h"
#include "ns3-dev/ns3/ndnSIM/utils/ndn-rtt-mean-deviation.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-wire.h"

#include <stdio.h>

#include <sstream>

#include "../utils/ndnsegmentstatus.h"
#include "../utils/congestionwindow.h"
#include "../utils/staticcongestionwindow.h"


#define WINDOW_MAX_RTO 1.0
#define STATS_OUTPUT_TIMER_MS 10
#define DEFAULT_MEAN_RTT_MS 100

namespace ns3
{
  namespace utils
  {
    class WindowNDNDownloader : public IDownloader, public ndn::App
    {
    public:
      WindowNDNDownloader();

      /* download Segment - calls download(URI) */
      virtual bool download (Segment *s);

      /* download from URI */
      virtual bool download (std::string URI);

      /* download segment before - not implemented here */
      virtual bool downloadBefore(Segment *s, int miliSeconds);

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void OnData (Ptr<const ndn::Data> contentObject);

      // (overriden from ndn::App) Call that will be called when a NACK arrives
      virtual void OnNack (Ptr<const ndn::Interest> interest);

      // (overridden from ndn::App) Processing upon start of the application
      virtual void StartApplication ();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void StopApplication ();

      virtual void setNodeForNDN (Ptr<Node> node);

      // get the physical available bitrate
      virtual uint64_t getPhysicalBitrate();


    protected:
      virtual void downloadChunk(int chunk_number);
      virtual Ptr<ndn::Interest> prepareInterestForDownload(int chunk_number);

      virtual void ScheduleNextChunkDownload();

      virtual void CancelAllTimeoutEvents();

      Ptr<ndn::RttEstimator> m_rtt;

      CongestionWindow cwnd;
      NDNSegmentStatus curSegmentStatus;

      unsigned int packets_received;
      unsigned int packets_received_this_second;
      unsigned int packets_sent_this_second;
      unsigned int packets_timeout;
      int packets_inflight;
      unsigned int packets_nack;


      bool had_nack;
      bool had_timeout;
      bool had_ack;

      EventId scheduleDownloadTimer;

      EventId resetStatisticsTimer;

      EventId statsOutputTimer;

      Time lastTimeout;
      Time lastValidPacket;

      void resetStatistics();
      void collectStats();

      void CheckRetrieveTimeout(int c_chunk_number);
      void OnTimeout (int c_chunk_number);

      int GetNextNeededChunkNumber();
      int GetNextNeededChunkNumber(int start_chunk_number);
    };
  }
}
#endif // WINDOWNDNDOWNLOADER_H
