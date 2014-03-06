#ifndef SVCADAPTIVESTRATEGY_H
#define SVCADAPTIVESTRATEGY_H

#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-l3-protocol.h"

#include "ns3-dev/ns3/ndn-fib.h"
#include "ns3-dev/ns3/ndn-fib-entry.h"
#include "ns3-dev/ns3/ndn-pit-entry.h"
#include "ns3-dev/ns3/ndn-interest.h"

#include "../../../ns-3/src/ndnSIM/model/fw/best-route.h"
#include "../../../ns-3/src/ndnSIM/model/fw/flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/smart-flooding.h"
#include "../../../ns-3/src/ndnSIM/model/fw/per-out-face-limits.h"

#include "../utils/ndntracer.h"

#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::SVCAdaptiveStrategy
template<class Parent>
class SVCAdaptiveStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SVCAdaptiveStrategy () {tracer = NULL;}

  virtual void AddFace(Ptr<Face> face);
  virtual void RemoveFace(Ptr<Face> face);

  virtual void OnInterest(Ptr< Face > face, Ptr< Interest > interest);
  virtual void DidSendOutInterest (Ptr<Face> inFace, Ptr<Face> outFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);

protected:
  static LogComponent g_log;
private:
  utils::NDNTracer* tracer;
};

template<class Parent>
LogComponent SVCAdaptiveStrategy<Parent>::g_log = LogComponent (SVCAdaptiveStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId SVCAdaptiveStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SVCAdaptiveStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SVCAdaptiveStrategy> ();
  return tid;
}

template<class Parent>
std::string SVCAdaptiveStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".SVCAdaptiveStrategy";
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::AddFace (Ptr<Face> face)
{
  if(tracer == NULL)
    tracer = new utils::NDNTracer(this);

  tracer->addFace (face);

  super::AddFace(face);
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::RemoveFace (Ptr<Face> face)
{
  if(tracer != NULL)
    tracer->removeFace(face);

  super::AddFace(face);
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::OnInterest (Ptr< Face > face, Ptr< Interest > interest)
{
  super::OnInterest(face,interest);
  fprintf(stderr, "getAvgInTrafficKbits = %d\n", tracer->getAvgInTrafficKbits (face));
  fprintf(stderr, "getAvgOutTrafficKbits = %d\n", tracer->getAvgOutTrafficKbits (face));
}

template<class Parent>
void SVCAdaptiveStrategy<Parent>::DidSendOutInterest (Ptr<Face> inFace, Ptr<Face> outFace,
                                        Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  //fprintf(stderr, "SVCAdaptiveStrategy: send out interest: %s\n", interest->GetName().toUri().c_str());
  super::DidSendOutInterest (inFace, outFace, interest, pitEntry);
  //todo
}

} // namespace fw
} // namespace ndn
} // namespace ns3
#endif // SVCADAPTIVESTRATEGY_H
