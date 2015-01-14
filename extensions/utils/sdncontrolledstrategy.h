#ifndef SDNCONTROLLEDSTRATEGY_H
#define SDNCONTROLLEDSTRATEGY_H

#include "ns3-dev/ns3/log.h"

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/ndn-wire.h"

#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-fib-limits.h"
#include "../../../ns-3/src/ndnSIM/model/fw/nacks.h"

#include "sdncontroller.h"

#include "boost/foreach.hpp"

#include <stdio.h>

#include "forwardingengine.h"

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SDNControlledStrategy
class SDNControlledStrategy: public ForwardingStrategy
{

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SDNControlledStrategy ();

  virtual void AddFace(Ptr< Face> face);
  virtual void RemoveFace(Ptr< Face > face);
  virtual void OnInterest(Ptr< Face > inFace, Ptr< Interest > interest);
  virtual bool DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
  virtual void WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
  virtual void WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
  virtual void DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);
  virtual void DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry);
  virtual bool TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);
  virtual void DidExhaustForwardingOptions(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);

  void init();

  Ptr<Face> GetFaceFromSDNController(Ptr<const Interest> interest);
  Ptr<Face> SelectFaceFromLocalFib(Ptr<const Interest> interest);

  Ptr<Interest> prepareNack(Ptr<const Interest> interest);

protected:

  int m_maxLevel;
  static LogComponent g_log;

  std::vector<Ptr<ndn::Face> > faces;
  Ptr<utils::ForwardingEngine> fwEngine;

  std::vector<std::map<Ptr<Name>, std::vector<Ptr<Face> > > > localFib;

  unsigned int prefixComponentNum;
  unsigned int useTockenBucket;
  bool initialized;
};

}
}
}

#endif // SDNCONTROLLEDSTRATEGY_H
