#include "forwardingstatistics.h"

using namespace ns3::ndn;

ForwardingStatistics::ForwardingStatistics(std::vector<int> faceIds)
{
  this->faceIds = faceIds;

  // initalize
  for(int i=0; i < MAX_LAYERS; i ++) // for each layer
  {
    for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
    {
      //stats[i] = ForwardingLayerStats();
      stats[i].unstatisfied_requests[*it] = 0;
      stats[i].statisfied_requests[*it] = 0;
      stats[i].goodput_bytes_received[*it] = 0;

      stats[i].last_goodput[*it] = 0;
      stats[i].last_reliability[*it] = 0;
    }
  }
  resetStatistics();
}

// here we log all unstatisfied requests. events like timeout and NACKs
void ForwardingStatistics::logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry, int ilayer)
{
  //fprintf(stderr, "TIMEOUT\n");
  //bytes_transmitted += pitEntry->GetInterest ()->GetWire ()->GetSize ();

  for (pit::Entry::out_container::iterator face = pitEntry->GetOutgoing ().begin ();face != pitEntry->GetOutgoing ().end (); face ++)
  {
    int index = getMapIndexFromFaceID ((*face).m_face->GetId ());
    stats[ilayer].unstatisfied_requests[index] += 1;
  }
}

//here we log all statisfied requests
void ForwardingStatistics::logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry, int ilayer)
{
  int index = getMapIndexFromFaceID (inFace->GetId ());
  stats[ilayer].statisfied_requests[index] += 1;
  stats[ilayer].goodput_bytes_received[index] += 4096; // TODO get real size
}

//here we log all events were we said forward on face x but e.g. per-out-face-limits revises our decision.
void ForwardingStatistics::logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace, int ilayer)
{
  //fprintf(stderr, "EXHAUSTED\n");
  int index = getMapIndexFromFaceID (targetedOutFace->GetId ());
  stats[ilayer].unstatisfied_requests[index] += 1;
}

void ForwardingStatistics::resetStatistics ()
{
  //fprintf(stderr, "\nNew Node:\n");
  for(int i=0; i < MAX_LAYERS; i ++) // for each layer
  {
    stats[i].last_goodput.clear ();
    stats[i].last_reliability.clear();
    stats[i].unstatisfied_traffic_fraction = 0;

    calculateLinkReliabilities (i);
    calculateGoodput(i);
    calculateUnstatisfiedTrafficFraction (i);

    stats[i].unstatisfied_requests.clear ();
    stats[i].statisfied_requests.clear ();
    stats[i].goodput_bytes_received.clear ();

    for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
    {
      stats[i].unstatisfied_requests[*it] = 0;
      stats[i].statisfied_requests[*it] = 0;
      stats[i].goodput_bytes_received[*it] = 0;
    }
  }
}

void ForwardingStatistics::calculateUnstatisfiedTrafficFraction(int layer)
{
  stats[layer].unstatisfied_traffic_fraction = 1.0;

  //sum up total goodput
  double total_goodput = 0.0;

  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    total_goodput += getGoodput(*it, layer);
  }

  if(total_goodput < 0.1) // goodput == 0
  {
    stats[layer].unstatisfied_traffic_fraction = 0;
    //fprintf(stderr, "layer[%d] no goodput measured\n", layer);
    return;
  }

  // for each face we determine the unstatisfied traffic fraction and normalize it finally
  double fraction = 0;
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    fraction += getLinkReliability(*it,layer) * (getGoodput (*it,layer) / total_goodput);
  }

  //fprintf(stderr, "layer[%d] unstatisfied traffic over all faces = %f\n", layer,1-fraction);

  stats[layer].unstatisfied_traffic_fraction -= fraction;
}

void ForwardingStatistics::calculateLinkReliabilities(int layer)
{
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
  {
    if(stats[layer].unstatisfied_requests[*it] == 0)
      stats[layer].last_reliability[*it] = 1.0;
    else
      stats[layer].last_reliability[*it] =
          (double)stats[layer].statisfied_requests[*it] / (double)(stats[layer].unstatisfied_requests[*it] + stats[layer].statisfied_requests[*it]);
  }
}

void ForwardingStatistics::calculateGoodput(int layer)
{
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
  {
    double bits = stats[layer].goodput_bytes_received[*it]*8;
    stats[layer].last_goodput[*it] = bits / 1000 / UPDATE_INTERVALL; // kbits;
  }
}

int ForwardingStatistics::getMapIndexFromFaceID(int face_id)
{
  for(int i = 0; i < faceIds.size (); i++)
  {
    if(faceIds.at(i) == face_id)
      return i;
  }

  NS_LOG_UNCOND("Error in getMapIndexFromFaceID");
  return -1;
}

double ForwardingStatistics::getLinkReliability(int face_id, int layer)
{
  int index = getMapIndexFromFaceID (face_id);
  return stats[layer].last_reliability[index];
}

double ForwardingStatistics::getGoodput(int face_id, int layer)
{
  int index = getMapIndexFromFaceID (face_id);
  return stats[layer].last_goodput[index];
}
