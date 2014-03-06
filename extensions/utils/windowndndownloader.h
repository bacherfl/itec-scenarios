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

#include <ns3-dev/ns3/ndn-l3-protocol.h>

#include <stdio.h>

#include <sstream>

#include "../utils/ndnsegmentstatus.h"

#include "../utils/congestionwindow.h"


#define WINDOW_MAX_RTO 1.0


namespace ns3
{
  namespace utils
  {
    class WindowNDNDownloader : public IDownloader, public ndn::App
    {
    public:
      WindowNDNDownloader();

      virtual bool download (Segment *s);

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void OnData (Ptr<const ndn::Data> contentObject);

      // (overriden from ndn::App) Call that will be called when a NACK arrives
      virtual void OnNack (Ptr<const ndn::Interest> interest);

      // (overridden from ndn::App) Processing upon start of the application
      virtual void StartApplication ();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void StopApplication ();

      virtual void setNodeForNDN (Ptr<Node> node);


    protected:
      virtual void downloadChunk(int chunk_number);

      virtual void ScheduleNextChunkDownload();


      Ptr<ndn::RttEstimator> m_rtt;

      CongestionWindow cwnd;
      NDNSegmentStatus curSegmentStatus;

      unsigned int packets_received;
      unsigned int packets_timeout;
      int packets_inflight;
      unsigned int packets_nack;


      EventId scheduleDownloadTimer;


      void CheckRetrieveTimeout(int c_chunk_number);
      void OnTimeout (int c_chunk_number);

      int GetNextNeededChunkNumber();
      int GetNextNeededChunkNumber(int start_chunk_number);
    };
  }
}
#endif // WINDOWNDNDOWNLOADER_H