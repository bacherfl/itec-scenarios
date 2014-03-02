#ifndef TIMEOUTNDNDOWNLOADER_H
#define TIMEOUTNDNDOWNLOADER_H

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

#include "ns3-dev/ns3/ndnSIM/utils/ndn-rtt-estimator.h"
#include "ns3-dev/ns3/ndnSIM/utils/ndn-rtt-mean-deviation.h"

#include <ns3-dev/ns3/ndn-l3-protocol.h>

#include <stdio.h>

#include <sstream>

namespace ns3
{
  namespace utils
  {
    /** this class represents the SimpleNDNDownloader enhanced with a Timeout and an RTO estimator */
    class TimeoutNDNDownloader : public IDownloader, public ndn::App
    {
    public:
      TimeoutNDNDownloader();

      virtual bool download (Segment *s);

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

      Ptr<ndn::RttEstimator> m_rtt;
      unsigned int packets_received;
      unsigned int packets_timeout;

      int packets_inflight;

      /** the timeoutEvent is set in downloadChunk() and canceled in onData() if packets are received.
        if no packets were received, it fires the CheckRetrieveTimeout which will then fire OnTimeout() */
      EventId timeoutEvent;

      void CheckRetrieveTimeout(int c_chunk_number);
      void OnTimeout (int c_chunk_number);

    };
  }
}

#endif // TIMEOUTNDNDOWNLOADER_H
