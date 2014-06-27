#ifndef RTTBASEDCACHINGSTRATEGY_H
#define RTTBASEDCACHINGSTRATEGY_H

#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/network-module.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-fib.h"
#include "ns3-dev/ns3/ndn-fib-entry.h"
#include "ns3-dev/ns3/ndn-pit-entry.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"

#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"

#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::RTTBasedCachingStrategy
template<class Parent>
class RTTBasedCachingStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  RTTBasedCachingStrategy () : super()
  {
    fprintf(stderr, "RTTBasedCachingStrategy activated...\n");
  }

  virtual void OnData( Ptr< Face > face, Ptr< Data > data);

protected:
  int m_maxLevel;
  static LogComponent g_log;
};

template<class Parent>
LogComponent RTTBasedCachingStrategy<Parent>::g_log = LogComponent (RTTBasedCachingStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId RTTBasedCachingStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::RTTBasedCachingStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <RTTBasedCachingStrategy> ();
  return tid;
}

template<class Parent>
std::string RTTBasedCachingStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".RTTBasedCachingStrategy";
}

template<class Parent>
void RTTBasedCachingStrategy<Parent>::OnData (Ptr<Face> inFace, Ptr<Data> data)
{
  /*NS_LOG_FUNCTION (inFace << data->GetName ());
  RTTBasedCachingStrategy<Parent>::m_inData (data, inFace);

  // Lookup PIT entry
  Ptr<pit::Entry> pitEntry = RTTBasedCachingStrategy<Parent>::m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (RTTBasedCachingStrategy<Parent>::m_cacheUnsolicitedData ||
          (RTTBasedCachingStrategy<Parent>::m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          // Optimistically add or update entry in the content store
          cached = RTTBasedCachingStrategy<Parent>::m_contentStore->Add (data);
        }
      else
        {
          // Drop data packet if PIT entry is not found
          // (unsolicited data packets should not "poison" content store)

          //drop dulicated or not requested data packet
          RTTBasedCachingStrategy<Parent>::m_dropData (data, inFace);
        }

      RTTBasedCachingStrategy<Parent>::DidReceiveUnsolicitedData (inFace, data, cached);
      return;
    }
  else
    {
      bool cached = RTTBasedCachingStrategy<Parent>::m_contentStore->Add (data);
      RTTBasedCachingStrategy<Parent>::DidReceiveSolicitedData (inFace, data, cached);
    }

  while (pitEntry != 0)
    {
      // Do data plane performance measurements
      RTTBasedCachingStrategy<Parent>::WillSatisfyPendingInterest (inFace, pitEntry);

      // Actually satisfy pending interest
      RTTBasedCachingStrategy<Parent>::SatisfyPendingInterest (inFace, data, pitEntry);

      // Lookup another PIT entry
      pitEntry = RTTBasedCachingStrategy<Parent>::m_pit->Lookup (*data);
    }*/
}

}
}
}
#endif // RTTBasedCachingStrategy


