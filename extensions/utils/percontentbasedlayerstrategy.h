#ifndef PERCONTENTBASEDLAYERSTRATEGY_H
#define PERCONTENTBASEDLAYERSTRATEGY_H

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

#include <stdio.h>

#include "forwardingengine.h"

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::ANYFORWARDINGSTRATEGY::PerContentBasedLayerStrategy
template<class Parent>
class PerContentBasedLayerStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  PerContentBasedLayerStrategy () : super()
  {
    fprintf(stderr, "PerContentBasedLayerStrategy activated...\n");
  }

  virtual void AddFace(Ptr< Face> face);
  virtual void RemoveFace(Ptr< Face > face);
  virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);
  virtual bool DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
  virtual void WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
  virtual void WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
  virtual void DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);

protected:

  int m_maxLevel;
  static LogComponent g_log;

  std::vector<Ptr<ndn::Face> > faces;
  Ptr<ForwardingEngine> fwEngine;
};

template<class Parent>
LogComponent PerContentBasedLayerStrategy<Parent>::g_log = LogComponent (PerContentBasedLayerStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId PerContentBasedLayerStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::PerContentBasedLayerStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <PerContentBasedLayerStrategy> ();
  return tid;
}

template<class Parent>
std::string PerContentBasedLayerStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".PerContentBasedLayerStrategy";
}

/* add face */
template<class Parent>
void PerContentBasedLayerStrategy<Parent>::AddFace (Ptr<Face> face)
{
  // add face to faces vector
  faces.push_back (face);

  fwEngine = new ForwardingEngine(faces);

  super::AddFace(face);
}

/* remove face */
template<class Parent>
void PerContentBasedLayerStrategy<Parent>::RemoveFace (Ptr<Face> face)
{
  // remove face from faces vector
  for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    if (face->GetId ()== (*it)->GetId())
    {
      faces.erase (it);
      break;
    }
  }

  fwEngine = new ForwardingEngine(faces);
  super::RemoveFace(face);
}


template<class Parent>
void PerContentBasedLayerStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{
  //todo
  interest->SetInterestLifetime (Time::FromDouble (0.5,Time::S));

  if(interest->GetNack () == Interest::NORMAL_INTEREST)
  {
    //fprintf(stderr, "ReceivedRequest for %s on Face %d\n", interest->GetName ().toUri().c_str(), inface->GetId ());
    super::OnInterest(inface,interest);
  }
  else
  {
    //fprintf(stderr, "ReceivedNACK for %s on Face %d\n", interest->GetName ().toUri().c_str(), inface->GetId ());

    Ptr<pit::Entry> pitEntry = PerContentBasedLayerStrategy<Parent>::m_pit->Lookup (*interest);
    if (pitEntry == 0)
    {
      // somebody is doing something bad
      PerContentBasedLayerStrategy<Parent>::m_dropNacks (interest, inface);
      fprintf(stderr, "Invalid NACK message\n");
      return;
    }

    fwEngine->logUnstatisfiedRequest (pitEntry);
    //TODO something useful here....
    pitEntry->ClearOutgoing ();
  }
}

template<class Parent>
bool PerContentBasedLayerStrategy<Parent>::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  int fwFaceId = fwEngine->determineRoute(inFace, interest);

  if(fwFaceId == DROP_FACE_ID)
  {
    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (ndn::Interest::NACK_CONGESTION);
    inFace->SendInterest (nack);

    PerContentBasedLayerStrategy<Parent>::m_outNacks (nack, inFace);
    fwEngine->logDroppingFace(inFace, interest, pitEntry);
    return false;
  }

  for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    if (fwFaceId == (*it)->GetId())
    {
      bool success = super::TrySendOutInterest(inFace, *it, interest, pitEntry);

      if(!success)
      {
        fwEngine->logExhaustedFace(inFace,interest,pitEntry, *it); /*means PerOutFaceLimits blocked it*/
      }

      return success; /*maybe some more sophisticated handling here...*/
    }
  }

  NS_LOG_UNCOND("Unhandeld Forwarding case!");
  return false;
}

template<class Parent>
void PerContentBasedLayerStrategy<Parent>::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  fwEngine->logUnstatisfiedRequest (pitEntry);
  super::WillEraseTimedOutPendingInterest(pitEntry);
}

template<class Parent>
void PerContentBasedLayerStrategy<Parent>::WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  if(inFace != 0) // ==0 means data comes from cache
    fwEngine->logStatisfiedRequest(inFace,pitEntry);

  super::WillSatisfyPendingInterest(inFace,pitEntry);
}

template<class Parent>
void PerContentBasedLayerStrategy<Parent>::DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  //fprintf(stderr, "SendOut %s on Face %d\n", interest->GetName ().toUri().c_str(), outFace->GetId ());
  super::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}

}
}
}

#endif // PERCONTENTBASEDLAYERSTRATEGY_H
