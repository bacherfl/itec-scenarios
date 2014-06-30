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
  //virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);
  virtual bool DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
  virtual void WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
  virtual void WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);

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


/*template<class Parent>
void PerContentBasedLayerStrategy<Parent>::OnInterest (Ptr< Face > inface, Ptr< Interest > interest)
{
  //SVCLevelTag levelTag;

  // lookup pit entry for interest (if exists)
  Ptr<pit::Entry> pitEntry = PerContentBasedLayerStrategy<Parent>::m_pit->Find (interest->GetName ());

  // check if duplicate interest first
  /bool isDuplicate = false;
  //if (pitEntry != 0)
  //{
  //  isDuplicate = pitEntry->IsNonceSeen (interest->GetNonce () );
  //}

  // means we have not seen a request for this chunk, so consider it for our statistics
  if(pitEntry != 0)
  {

  }
  // let parent continue with the interest
  super::OnInterest(inface,interest);
}*/

template<class Parent>
bool PerContentBasedLayerStrategy<Parent>::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  int fwFaceId = fwEngine->determineRoute(inFace, interest);

  for (std::vector<Ptr<ndn::Face> >::iterator it = faces.begin ();
      it !=  faces.end (); ++it)
  {
    if (fwFaceId == (*it)->GetId())
    {
      bool success = super::TrySendOutInterest(inFace, *it, interest, pitEntry);

      if(!success)
        fwEngine->logExhaustedFace(inFace,interest,pitEntry, *it);

      return success; /*maybe some more sophisticated handling here...*/
    }
  }

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

}
}
}

#endif // PERCONTENTBASEDLAYERSTRATEGY_H
