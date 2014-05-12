#ifndef NDNSEGMENTSTATUS_H
#define NDNSEGMENTSTATUS_H


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



#include "ns3-dev/ns3/event-id.h"

#include <ns3-dev/ns3/ndn-l3-protocol.h>

#include <stdio.h>

#include <sstream>


namespace ns3
{
  enum NDNDownloadStatus { NotInitiated= 0, Requested= 1, Received= 2, Timeout=3, Aborted=4 };


  class NDNSegmentStatus
  {
  public:
    /** the base URI of the resource that is being requested */
    std::string base_uri;

    /** the amount of chunks we are expecting */
    unsigned int num_chunks;

    /** an Array with the status of each chunk */
    std::vector<NDNDownloadStatus> chunk_status;
    //NDNDownloadStatus* chunk_status;

    /** an int value that states how many bytes we still have to download */
    int bytesToDownload;

    /** an Array with timeout events, so we can cancel them if needed */
    //std::vector<EventId> chunk_timeout_events;

    EventId chunk_timeout_events[4096];
    //EventId* chunk_timeout_events;

    /** the average bitrate of the current representation/quality-level */
    unsigned int avgBitrate;
  };
}




#endif // NDNSEGMENTSTATUS_H
