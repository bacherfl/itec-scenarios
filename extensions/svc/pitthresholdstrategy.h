#ifndef PITTHRESHOLDSTRATEGY_H
#define PITTHRESHOLDSTRATEGY_H

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

#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>

#include <stdio.h>
#include <vector>

#include "svcleveltag.h"
#include "svcbitratetag.h"
#include "facepitstatistic.h"

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::PitThresholdStrategy
template<class Parent>
class PitThresholdStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  virtual void AddFace(Ptr<Face> face);
  virtual void RemoveFace(Ptr<Face> face);

  PitThresholdStrategy (){}

protected:

  //An event that is fired every time a new PIT entry is created.
  virtual void DidCreatePitEntry(Ptr< Face > inFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);

  //Event fired just before PIT entry is removed by timeout.
  virtual void 	WillEraseTimedOutPendingInterest (Ptr< pit::Entry > pitEntry);

  //Event which is fired just after data was send out on the face.
  virtual void 	DidSendOutData (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Data > data, Ptr< pit::Entry > pitEntry);

  //Event fired just after forwarding the Interest.
  virtual void DidSendOutInterest ( Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry );

  //Event which is fired every time a requested (solicited) DATA packet (there is an active PIT entry) is received.
  virtual void DidReceiveSolicitedData(Ptr< Face > inFace, Ptr< const Data > data, bool didCreateCacheEntry);

  virtual void OnInterest (Ptr< Face > face, Ptr< Interest > interest);

  typedef boost::unordered_map<int/*faceID*/, svc::FacePITStatistic*> FacePITStatisticMap;
  FacePITStatisticMap map;

  std::vector<Ptr<ndn::Face> > faces;

  static LogComponent g_log;
};

template<class Parent>
LogComponent PitThresholdStrategy<Parent>::g_log = LogComponent (PitThresholdStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId PitThresholdStrategy<Parent>::GetTypeId ()
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::PitThresholdStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <PitThresholdStrategy> ();
  return tid;
}

template<class Parent>
std::string PitThresholdStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".PitThresholdStrategy";
}

template<class Parent>
void PitThresholdStrategy<Parent>::AddFace (Ptr<Face> face)
{
  PitThresholdStrategy<Parent>::map[face->GetId ()] = new svc::FacePITStatistic(face);
  PitThresholdStrategy<Parent>::faces.push_back (face);
  super::AddFace(face);
}

template<class Parent>
void PitThresholdStrategy<Parent>::RemoveFace (Ptr<Face> face)
{
  svc::FacePITStatistic* p = PitThresholdStrategy<Parent>::map[face->GetId ()];

  PitThresholdStrategy<Parent>::map.erase(face->GetId ());

  if(p!=NULL)
    delete p;

  for(std::vector<Ptr<ndn::Face> >::iterator it = PitThresholdStrategy<Parent>::faces.begin ();
      it !=  PitThresholdStrategy<Parent>::faces.end (); ++it)
  {
    if(face== (*it))//todo
    {
      PitThresholdStrategy<Parent>::faces.erase (it);
      break;
    }
  }
  super::RemoveFace(face);
}

template<class Parent>
void PitThresholdStrategy<Parent>::DidCreatePitEntry(Ptr< Face > inFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  //increase for outgoing face too (see DidSendOutInterest) !
  PitThresholdStrategy<Parent>::map[inFace->GetId ()]->increase(); //this logs all pending interests for incoming faces
  super::DidCreatePitEntry(inFace, interest, pitEntry);
}


template<class Parent>
void PitThresholdStrategy<Parent>::WillEraseTimedOutPendingInterest (Ptr< pit::Entry > pitEntry)
{
  /*erase stats for all in and outgoing faces on timeiout*/

  std::set<ndn::pit::OutgoingFace> outfaces = pitEntry->GetOutgoing ();
  std::set<ndn::pit::IncomingFace> infaces = pitEntry->GetIncoming ();

  for(std::set<ndn::pit::OutgoingFace>::iterator outIt = outfaces.begin (); outIt != outfaces.end (); ++outIt)
  {
    ndn::pit::OutgoingFace f = *outIt; /*bad but needed since outfaces are const :(*/

    for(std::vector<Ptr<ndn::Face> >::iterator it = PitThresholdStrategy<Parent>::faces.begin ();
        it !=  PitThresholdStrategy<Parent>::faces.end (); ++it)
    {
      if(f == *it)
      {
        PitThresholdStrategy<Parent>::map[(*it)->GetId ()]->decrease();
        break;
      }
    }
  }

  for(std::set<ndn::pit::IncomingFace>::iterator inIt = infaces.begin (); inIt != infaces.end (); ++inIt)
  {
    ndn::pit::IncomingFace f = *inIt; /*bad but needed since outfaces are const :(*/
    for(std::vector<Ptr<ndn::Face> >::iterator it = PitThresholdStrategy<Parent>::faces.begin ();
        it !=  PitThresholdStrategy<Parent>::faces.end (); ++it)
    {
      if(f == *it)
      {
        PitThresholdStrategy<Parent>::map[(*it)->GetId ()]->decrease();
        break;
      }
    }
  }

  super::WillEraseTimedOutPendingInterest(pitEntry);
}

template<class Parent>
void PitThresholdStrategy<Parent>::DidSendOutData (Ptr< Face > inFace/*inc face of data*/, Ptr< Face > outFace/*outgoing face of data*/,
                                                   Ptr< const Data > data, Ptr< pit::Entry > pitEntry)
{
  PitThresholdStrategy<Parent>::map[outFace->GetId ()]->decrease();
  //PitThresholdStrategy<Parent>::map[inFace-> GetId ()]->decrease(); // do this in DidReceiveSolicitedData
  super::DidSendOutData(inFace, outFace, data, pitEntry);
}

template<class Parent>
void PitThresholdStrategy<Parent>::DidReceiveSolicitedData(Ptr< Face > inFace, Ptr< const Data > data, bool didCreateCacheEntry)
{
  PitThresholdStrategy<Parent>::map[inFace-> GetId ()]->decrease();
  super::DidReceiveSolicitedData(inFace, data, didCreateCacheEntry);
}

template<class Parent>
void PitThresholdStrategy<Parent>::DidSendOutInterest ( Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry )
{
  PitThresholdStrategy<Parent>::map[outFace->GetId ()]->increase(); //this logs all pending interests that have been send out on outfaces
  super::DidSendOutInterest(inFace, outFace, interest, pitEntry);
}

template<class Parent>
void PitThresholdStrategy<Parent>::OnInterest (Ptr< Face > face, Ptr< Interest > interest)
{
  super::OnInterest(face,interest);
}

} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // PITTHRESHOLDSTRATEGY_H
