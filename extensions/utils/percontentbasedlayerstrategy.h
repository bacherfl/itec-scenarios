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
#include "../../../ns-3/src/ndnSIM/model/fw/per-fib-limits.h"
#include "../../../ns-3/src/ndnSIM/model/fw/nacks.h"

#include "boost/foreach.hpp"

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
    //fprintf(stderr, "PerContentBasedLayerStrategy activated...\n");

    fwEngine = new utils::ForwardingEngine(faces, PerContentBasedLayerStrategy<Parent>::m_fib, prefixComponentNum);
  }

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

  Ptr<Interest> prepareNack(Ptr<const Interest> interest);

protected:

  int m_maxLevel;
  static LogComponent g_log;

  std::vector<Ptr<ndn::Face> > faces;
  Ptr<utils::ForwardingEngine> fwEngine;

  unsigned int prefixComponentNum;
  unsigned int useTockenBucket;
};

template<class Parent>
LogComponent PerContentBasedLayerStrategy<Parent>::g_log = LogComponent (PerContentBasedLayerStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId PerContentBasedLayerStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::PerContentBasedLayerStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <PerContentBasedLayerStrategy> ()
      .template AddAttribute("PrefixNameComponentIndex", "The component of the name that is considered as prefix",
                    IntegerValue(0),
                    MakeIntegerAccessor(&PerContentBasedLayerStrategy<Parent>::prefixComponentNum),
                             MakeIntegerChecker<int32_t>())
      .template AddAttribute("UseTokenBucket", "Wether to use a Token Bucket Filter per Content Prefix. 1 = Use, 0 = do not use",
                    IntegerValue(1),
                    MakeIntegerAccessor(&PerContentBasedLayerStrategy<Parent>::useTockenBucket),
                             MakeIntegerChecker<int32_t>());
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
  fwEngine = new utils::ForwardingEngine(faces, PerContentBasedLayerStrategy<Parent>::m_fib, prefixComponentNum);
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

  fwEngine = new utils::ForwardingEngine(faces, PerContentBasedLayerStrategy<Parent>::m_fib, prefixComponentNum);
  super::RemoveFace(face);
}

bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

template<class Parent>
void PerContentBasedLayerStrategy<Parent>::OnInterest (Ptr< Face > inFace, Ptr< Interest > interest)
{
  super::OnInterest(inFace,interest);

  /*if(interest->GetNack () == Interest::NORMAL_INTEREST)
  {
    super::OnInterest(inFace,interest);
  }
  else
  {
    Ptr<pit::Entry> pitEntry = PerContentBasedLayerStrategy<Parent>::m_pit->Lookup (*interest);
    if (pitEntry == 0)
    {
      PerContentBasedLayerStrategy<Parent>::m_dropNacks (interest, inFace);
      return;
    }
    fwEngine->logUnstatisfiedRequest (pitEntry);
    // we dont call super::NACK(), since we skip looking for other sources.

    // set all outgoing faces to useless (in vain)
    for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
        it !=  faces.end (); ++it)
    {
        if ((*it)->GetId() != inFace->GetId())
        {
          pitEntry->AddOutgoing ((*it));
          pitEntry->SetWaitingInVain ((*it));
        }
    }

    //forward nack
    Ptr<Interest> nack = Create<Interest> (*interest);
    nack->SetNack (interest->GetNack ());
    BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      incoming.m_face->SendInterest (nack);
      PerContentBasedLayerStrategy<Parent>::m_outNacks (nack, incoming.m_face);
    }

    pitEntry->RemoveIncoming (inFace);
    pitEntry->ClearOutgoing ();
  }*/

}

template<class Parent>
Ptr<Interest> PerContentBasedLayerStrategy<Parent>::prepareNack(Ptr<const Interest> interest)
{
  Ptr<Interest> nack = Create<Interest> (*interest);

  //nack->SetNack (ndn::Interest::NACK_CONGESTION);
  nack->SetNack (ndn::Interest::NACK_CONGESTION); // set this since ndn changes it anyway to this.

  /*SVCLevelTag levelTag; // this causes loops together with limits on the clients be careful
  levelTag.Set (-1); // means packet dropped on purpose
  nack->GetPayload ()->AddPacketTag (levelTag);*/

  //fprintf(stderr, "NACK %s prepared at time: %f\n", interest->GetName ().toUri ().c_str (), Simulator::Now ().ToDouble (Time::S));
  return nack;
}

template<class Parent>
bool PerContentBasedLayerStrategy<Parent>::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  //lets find the origin inface(s)
  std::vector<Ptr<Face> > originInFaces;
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
  {
    originInFaces.push_back(incoming.m_face);
  }

  //lets find the already tried faces
  std::vector<int> triedFaces;
  BOOST_FOREACH (const pit::OutgoingFace &outgoing, pitEntry->GetOutgoing())
  {
    if(std::find(originInFaces.begin (), originInFaces.end (), outgoing.m_face) == originInFaces.end ())
      triedFaces.push_back (outgoing.m_face->GetId());
  }

  //fprintf(stderr, "total(%lu),incoming(%lu),tried(%lu)\n", this->faces.size (), originInFaces.size (), triedFaces.size ());

  int fwFaceId = fwEngine->determineRoute(originInFaces, interest, pitEntry, triedFaces);

  while(fwFaceId != DROP_FACE_ID)
  {
    for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
        it !=  faces.end (); ++it)
    {
      if (fwFaceId == (*it)->GetId())
      {
        bool success = PerContentBasedLayerStrategy<Parent>::TrySendOutInterest(inFace, *it, interest, pitEntry);
        if(!success)
        {
          //fprintf(stderr, "Blocked! Interest %s blocked on Face %d\n", interest->GetName ().toUri ().c_str (), fwFaceId);
          triedFaces.push_back (fwFaceId);
          //fwEngine->logExhaustedFace (inFace, interest, pitEntry, *it); //logging here an error is not a good idea
          pitEntry->AddOutgoing ((*it));
          pitEntry->SetWaitingInVain ((*it));
          break;
        }
        //fprintf(stderr, "Successs! Interest %s send out on Face %d\n", interest->GetName ().toUri ().c_str (), fwFaceId);
        return success;
      }
    }

    fwFaceId = fwEngine->determineRoute(originInFaces, interest, pitEntry,triedFaces);
  }

  // set all outgoing faces to useless (in vain)
  for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
      if ((*it)->GetId() != inFace->GetId())
      {
        pitEntry->AddOutgoing ((*it));
        pitEntry->SetWaitingInVain ((*it));
      }
  }

  fwEngine->logDroppingFace(inFace, interest, pitEntry);
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

template<class Parent>
void PerContentBasedLayerStrategy<Parent>::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{
  fwEngine->logUnstatisfiedRequest (inFace, pitEntry);
  super::DidReceiveValidNack (inFace, nackCode, nack, pitEntry);
}

template<class Parent>
bool PerContentBasedLayerStrategy<Parent>::TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  if(useTockenBucket > 0)
  {
    if(!fwEngine->tryForwardInterest (outFace, interest))
      return false;
  }
  return super::TrySendOutInterest(inFace,outFace, interest, pitEntry);
}

template<class Parent>
void PerContentBasedLayerStrategy<Parent>::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  return super::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

}
}
}

#endif // PERCONTENTBASEDLAYERSTRATEGY_H
