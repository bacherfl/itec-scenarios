#ifndef SDNROUTER_H
#define SDNROUTER_H

#include <stdio.h>
#include <jsoncpp/json/json.h>

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


namespace ns3 {
namespace ndn {
namespace fw {

template<class Parent>
class SDNRouter : public Parent
{
    typedef Parent super;
public:
    SDNRouter () : super()
    {

    }

    static TypeId GetTypeId(void);

    virtual void OnData(Ptr<Face> face, Ptr<Data> data);

    virtual void AddFace(Ptr< Face> face);
    virtual void RemoveFace(Ptr< Face > face);
    virtual void OnInterest(Ptr< Face > inface, Ptr< Interest > interest);
    virtual bool DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);
    virtual void WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
    virtual void WillSatisfyPendingInterest (Ptr<Face> inFace, Ptr<pit::Entry> pitEntry);
    virtual void DidSendOutInterest (Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);
    virtual void DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry);
    virtual bool TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry);

};

template<class Parent>
TypeId SDNRouter<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::SDNRouter").c_str ())
      .SetGroupName ("Ndn")
      .template SetParent <super> ()
      .template AddConstructor <SDNRouter> ()
        ;
  return tid;
}

template<class Parent>
void SDNRouter<Parent>::OnData(Ptr<Face> face, Ptr<Data> data)
{
    std::cout << "Received data";

    //TODO move response handling to separate handler class
    //check for special controller prefix
    std::string prefix = data->GetName().getPrefix(1, 0).toUri();

    if (prefix.compare("/controller") == 0)
    {
        //received controller response
        uint8_t *buf = (uint8_t*)(malloc (sizeof(uint8_t) * data->GetPayload()->GetSize()));
        data->GetPayload()->CopyData(buf, data->GetPayload()->GetSize());
        std::string jsonString(reinterpret_cast<char const*>(buf));

        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(jsonString, root))
        {
            std::cout << "Error while parsing controller response";
            return;
        }

        const std::string controllerId = root.get("id", "unknown").asString();
        int seqNr = root.get("seq", 0).asInt();

        std::cout << "Found controller " << controllerId << " via face " << face->GetId() << ", seq=" << seqNr << "\n";
    }
}

template<class Parent>
void SDNRouter<Parent>::AddFace(Ptr<Face> face)
{
    super::AddFace(face);
}

template<class Parent>
void SDNRouter<Parent>::RemoveFace(Ptr<Face> face)
{
    super::RemoveFace(face);
}

template<class Parent>
bool SDNRouter<Parent>::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    typedef fib::FaceMetricContainer::type::index<fib::i_metric>::type FacesByMetric;
      FacesByMetric &faces = pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ();
      FacesByMetric::iterator faceIterator = faces.begin ();

      int propagatedCount = 0;

      // forward to best-metric face
      if (faceIterator != faces.end ())
        {
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
void SDNRouter<Parent>::OnInterest(Ptr< Face > inface, Ptr< Interest > interest)
{
    std::cout << "received interest \n";
    SDNRouter<Parent>::OnInterest(inface, interest);
}

template<class Parent>
void SDNRouter<Parent>::WillEraseTimedOutPendingInterest(Ptr<pit::Entry> pitEntry)
{
    SDNRouter<Parent>::WillEraseTimedOutPendingInterest(pitEntry);
}

template<class Parent>
void SDNRouter<Parent>::WillSatisfyPendingInterest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
    SDNRouter<Parent>::WillSatisfyPendingInterest(inFace, pitEntry);
}

template<class Parent>
void SDNRouter<Parent>::DidSendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
    SDNRouter<Parent>::DidSendOutInterest(inFace, outFace, interest, pitEntry);
}

template<class Parent>
void SDNRouter<Parent>::DidReceiveValidNack (Ptr<Face> inFace, uint32_t nackCode, Ptr<const Interest> nack, Ptr<pit::Entry> pitEntry)
{
    SDNRouter<Parent>::DidReceiveValidNack(inFace, nackCode, nack, pitEntry);
}

template<class Parent>
bool SDNRouter<Parent>::TrySendOutInterest(Ptr< Face > inFace, Ptr< Face > outFace, Ptr< const Interest > interest, Ptr< pit::Entry > pitEntry)
{
    SDNRouter<Parent>::TrySendOutInterest(inFace, outFace, interest, pitEntry);
}


} //namespace fw
} //namespace ndn
} //namespace ns3
#endif // SDNROUTER_H
