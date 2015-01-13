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
template<class Parent>
class SDNControlledStrategy: public Parent
{
  typedef Parent super;

public:
  static TypeId GetTypeId ();

  static std::string GetLogName ();

  SDNControlledStrategy () : super()
  {
    //fprintf(stderr, "SDNControlledStrategy activated...\n");

    fwEngine = new utils::ForwardingEngine(faces, SDNControlledStrategy<Parent>::m_fib, prefixComponentNum);
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
};

template<class Parent>
LogComponent SDNControlledStrategy<Parent>::g_log = LogComponent (SDNControlledStrategy<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId SDNControlledStrategy<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SDNControlledStrategy").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SDNControlledStrategy> ();
  return tid;
}

template<class Parent>
std::string SDNControlledStrategy<Parent>::GetLogName ()
{
  return super::GetLogName () + ".SDNControlledStrategy";
}

/* add face */
template<class Parent>
void SDNControlledStrategy<Parent>::AddFace (Ptr<Face> face)
{
  // add face to faces vector
  faces.push_back (face);
  fwEngine = new utils::ForwardingEngine(faces, SDNControlledStrategy<Parent>::m_fib, prefixComponentNum);
  super::AddFace(face);
}

/* remove face */
template<class Parent>
void SDNControlledStrategy<Parent>::RemoveFace (Ptr<Face> face)
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

  fwEngine = new utils::ForwardingEngine(faces, SDNControlledStrategy<Parent>::m_fib, prefixComponentNum);
  super::RemoveFace(face);
}

template<class Parent>
void SDNControlledStrategy<Parent>::OnInterest (Ptr< Face > inFace, Ptr< Interest > interest)
{
  super::OnInterest(inFace,interest);

  /*if(interest->GetNack () == Interest::NORMAL_INTEREST)
  {
    super::OnInterest(inFace,interest);
  }
  else
  {
    Ptr<pit::Entry> pitEntry = SDNControlledStrategy<Parent>::m_pit->Lookup (*interest);
    if (pitEntry == 0)
    {
      SDNControlledStrategy<Parent>::m_dropNacks (interest, inFace);
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
      SDNControlledStrategy<Parent>::m_outNacks (nack, incoming.m_face);
    }

    pitEntry->RemoveIncoming (inFace);
    pitEntry->ClearOutgoing ();
  }*/

}

template<class Parent>
Ptr<Interest> SDNControlledStrategy<Parent>::prepareNack(Ptr<const Interest> interest)
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
Ptr<Face> SDNControlledStrategy<Parent>::GetFaceFromSDNController(Ptr<const Interest> interest)
{
    //TODO implement method
    //SDNController::
    return NULL;
}

template<class Parent>
Ptr<Face> SDNControlledStrategy<Parent>::SelectFaceFromLocalFib(Ptr<const Interest> interest)
{
    //TODO implement method
    return NULL;
}

template<class Parent>
bool SDNControlledStrategy<Parent>::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    Ptr<Face> outFace = SelectFaceFromLocalFib(interest);

    if (outFace != NULL)
    {

    }
    else {
        outFace = GetFaceFromSDNController(interest);
    }
    typedef fib::FaceMetricContainer::type::index<fib::i_metric>::type FacesByMetric;
      FacesByMetric &faces = pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ();
      FacesByMetric::iterator faceIterator = faces.begin ();

      int propagatedCount = 0;

      // forward to best-metric face
      if (faceIterator != faces.end ())
        {
          std::cout << "test";
          if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
            propagatedCount ++;

          faceIterator ++;
        }

      // forward to second-best-metric face
      if (faceIterator != faces.end ())
        {
          if (TrySendOutInterest (inFace, faceIterator->GetFace (), interest, pitEntry))
            propagatedCount ++;

          faceIterator ++;
        }
      return propagatedCount > 0;
}

template<class Parent>
void SDNControlledStrategy<Parent>::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  super::WillEraseTimedOutPendingInterest(pitEntry);
}

template<class Parent>
void SDNControlledStrategy<Parent>::WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  if(inFace != 0) // ==0 means data comes from cache
    fwEngine->logStatisfiedRequest(inFace,pitEntry);

  super::WillSatisfyPendingInterest(inFace,pitEntry);
}

template<class Parent>
void SDNControlledStrategy<Parent>::DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  //fprintf(stderr, "SendOut %s on Face %d\n", interest->GetName ().toUri().c_str(), outFace->GetId ());
  super::DidSendOutInterest(inFace,outFace,interest,pitEntry);
}

template<class Parent>
void SDNControlledStrategy<Parent>::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{
  fwEngine->logUnstatisfiedRequest (inFace, pitEntry);
  super::DidReceiveValidNack (inFace, nackCode, nack, pitEntry);
}

template<class Parent>
bool SDNControlledStrategy<Parent>::TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
  if(useTockenBucket > 0)
  {
    if(!fwEngine->tryForwardInterest (outFace, interest))
      return false;
  }
  return super::TrySendOutInterest(inFace,outFace, interest, pitEntry);
}

template<class Parent>
void SDNControlledStrategy<Parent>::DidExhaustForwardingOptions (Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  return super::DidExhaustForwardingOptions (inFace, interest, pitEntry);
}

}
}
}

#endif // SDNCONTROLLEDSTRATEGY_H
