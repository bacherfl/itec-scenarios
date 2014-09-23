#include "customstrategy.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-fib-entry.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/core-module.h"
#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED(CustomStrategy);

//LogComponent CustomStrategy::g_log = LogComponent(CustomStrategy::GetLogName().c_str());
NS_LOG_COMPONENT_DEFINE ("ndn.fw.CustomStrategy");

CustomStrategy::CustomStrategy()
{
}

std::string
CustomStrategy::GetLogName()
{
    return "ndn.fw.CustomStrategy";
}

TypeId
CustomStrategy::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ndn::fw::CustomStrategy")
            .SetGroupName("Ndn")
            .SetParent<ForwardingStrategy>()
            .AddConstructor<CustomStrategy>();

    return tid;
}

bool CustomStrategy::DoPropagateInterest(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
    typedef fib::FaceMetricContainer::type::index<fib::i_metric>::type FacesByMetric;
    FacesByMetric &faces = pitEntry->GetFibEntry()->m_faces.get<fib::i_metric>();
    FacesByMetric::iterator faceIterator = faces.begin();

    typedef std::vector<Ptr<ndn::Face> > MyFaceList;

    int propagatedCount = 0;

    Ptr<Node> node = this->GetObject<Node>();

    std::string prefix = interest->GetName().getPrefix(2, 0).toUri();

    std::stringstream sentFaces;

    //if a known interface for the prefix exists, use this one
    if ((knownFaces.count(prefix) > 0))
    {
        std::set<Ptr<Face> > faces = knownFaces[prefix];
        //take the first one
        //Ptr<Face> face = *(faces.begin());
        double p = 1.0 - ((double)(1.0) / faces.size());
        double r = ((double) rand() / (RAND_MAX));
        NS_LOG_DEBUG("Random r = " << r);
        for (std::set<Ptr<Face > >::iterator it = faces.begin(); it != faces.end(); ++it)
        {
            Ptr<Face> face = *it;

            if ((face->GetId() != inFace->GetId()) && (ndn::Face::APPLICATION & face->GetFlags() == 0))
            {
                NS_LOG_DEBUG("p = " << p << ", |Known Faces| = " << faces.size());
                if (r >= p)
                {
                    if (TrySendOutInterest(inFace, face, interest, pitEntry))
                    {
                        sentFaces << face->GetId() << ", ";
                        propagatedCount++;
                        break;
                    }
                } else p = p - (double)(1.0) / faces.size();
            }
        }
    }

    //otherwise, flood the interest to all faces
    else
    {
        for (MyFaceList::iterator it = facesList.begin(); it != facesList.end(); ++it)
        {
            Ptr<Face> face = *it;
            if ((face->GetId() != inFace->GetId()) && (ndn::Face::APPLICATION & face->GetFlags() == 0))
            {
                if ((TrySendOutInterest(inFace, face, interest, pitEntry)))
                {
                    sentFaces << face->GetId() << ", ";
                    propagatedCount++;
                }
            }

        }
    }

    NS_LOG_DEBUG(Names::FindName(node) << " Sent Interest for " << interest->GetName().toUri() << " from Face " << inFace->GetId() << " to faces " << sentFaces.str());

    events[Simulator::Now().GetMilliSeconds()] = propagatedCount * interest->GetPayload()->GetSize();

    /*
    //forward to the best metric face
    if (faceIterator != faces.end())
    {
        if (TrySendOutInterest (inFace, faceIterator->GetFace(), interest, pitEntry))
            propagatedCount++;

        faceIterator++;
    }

    if (faceIterator != faces.end())
    {
        if (TrySendOutInterest (inFace, faceIterator->GetFace(), interest, pitEntry))
            propagatedCount++;

        faceIterator++;
    }
    */
    if (propagatedCount == 0)
    {
        if (faceIterator != faces.end())
        {
            if (TrySendOutInterest (inFace, faceIterator->GetFace(), interest, pitEntry))
                propagatedCount++;

            faceIterator++;
        }

        if (faceIterator != faces.end())
        {
            if (TrySendOutInterest (inFace, faceIterator->GetFace(), interest, pitEntry))
                propagatedCount++;

            faceIterator++;
        }
    }
    return propagatedCount > 0;
}

void CustomStrategy::AddFace(Ptr<Face> face)
{
    NS_LOG_DEBUG("Adding face " << face->GetId() << " on node " << face->GetNode());
    facesList.push_back(face);
    ForwardingStrategy::AddFace(face);
}

void CustomStrategy::OnData(Ptr<Face> face, Ptr<Data> data)
{
    Ptr<Node> node = this->GetObject<Node>();
    NS_LOG_DEBUG(Names::FindName(node) << "received " << data->GetName());
    std::string name = data->GetName().getPrefix(2, 0).toUri();

    if (knownFaces.count(name) == 0)
    {
        std::set<Ptr<Face> > newFacesList;
        newFacesList.insert(face);
        knownFaces[name] = newFacesList;
    }
    else {
        std::set<Ptr<Face> > faces = knownFaces[name];
        bool found = false;
        for (std::set<Ptr<Face> >::iterator it2 = faces.begin(); it2 != faces.end(); ++it2)
        {
            Ptr<Face> tmpFace = *it2;
            if (tmpFace->GetId() == face->GetId())
                found = true;
        }
        if (!found)
        {
            faces.insert(face);
            knownFaces[name] = faces;
        }
    }

    NS_LOG_DEBUG (Names::FindName(node) << " Adding face " << face->GetId() << " as known source for content " << name.c_str());
    std::stringstream strFacesList;

    for (FacesMap::iterator it = knownFaces.begin(); it != knownFaces.end(); ++it) {
        strFacesList << it->first << ": ";
        for (std::set<Ptr<Face> >::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            Ptr<Face> face = *it2;
            strFacesList << face->GetId() << ", ";
        }        

        NS_LOG_DEBUG (Names::FindName(node) << ": Known Faces for " << strFacesList.str());
    }


    ForwardingStrategy::OnData(face, data);
}

void CustomStrategy::writeLog()
{

}

}
}
}


