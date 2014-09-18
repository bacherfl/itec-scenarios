#include "forwardingstatistics.h"

using namespace ns3::ndn;

NS_LOG_COMPONENT_DEFINE ("ForwardingStatistics");

ForwardingStatistics::ForwardingStatistics(std::vector<int> faceIds)
{
  this->faceIds = faceIds;

  // initalize
  for(int i=0; i < MAX_LAYERS; i ++) // for each layer
  {
    for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
    {
      stats[i].unstatisfied_requests[*it] = 0;
      stats[i].statisfied_requests[*it] = 0;
      stats[i].goodput_bytes_received[*it] = 0;

      stats[i].last_goodput[*it] = 0;
      stats[i].last_reliability[*it] = 0;
      stats[i].last_actual_forwarding_probs[*it] = 0;
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
    stats[ilayer].unstatisfied_requests[(*face).m_face->GetId ()] += 1;
}

//here we log all statisfied requests
void ForwardingStatistics::logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry, int ilayer)
{
  //int index = getMapIndexFromFaceID (inFace->GetId ());
  stats[ilayer].statisfied_requests[inFace->GetId ()] += 1;
  stats[ilayer].goodput_bytes_received[inFace->GetId ()] += 4096; // TODO get real size for now its ok since we do not use the received bytes for something..
}

//here we log all events were we said forward on face x but e.g. per-out-face-limits revises our decision.
void ForwardingStatistics::logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace, int ilayer)
{
  stats[ilayer].unstatisfied_requests[targetedOutFace->GetId ()] += 1;
}

void ForwardingStatistics::logDroppingFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, int ilayer)
{
  stats[ilayer].statisfied_requests[DROP_FACE_ID] += 1;
}

void ForwardingStatistics::resetStatistics ()
{
  for(int i=0; i < MAX_LAYERS; i ++) // for each layer
  {
    stats[i].last_goodput.clear ();
    stats[i].last_reliability.clear();
    stats[i].unstatisfied_traffic_fraction = 0;
    stats[i].unstatisfied_traffic_fraction_reliable_faces = 0;
    stats[i].unstatisfied_traffic_fraction_unreliable_faces = 0;
    stats[i].last_actual_forwarding_probs.clear();

    calculatTotalForwardedRequests(i);
    calculateLinkReliabilities (i);
    calculateGoodput(i);
    calculateUnstatisfiedTrafficFraction (i);
    calculateUnstatisfiedTrafficFractionOfReliableFaces (i);
    calculateUnstatisfiedTrafficFractionOfUnreliableFaces (i);
    calculateActualForwardingProbabilities (i);

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

void ForwardingStatistics::calculatTotalForwardedRequests(int layer)
{
  stats[layer].total_forwarded_requests = 0;
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    stats[layer].total_forwarded_requests += stats[layer].unstatisfied_requests[*it] + stats[layer].statisfied_requests[*it];
  }
}

void ForwardingStatistics::calculateUnstatisfiedTrafficFraction(int layer)
{
  //sum up total goodput
  double total_interests =  stats[layer].total_forwarded_requests;

  double fraction = 0;
  if(total_interests < 0.1) // goodput == 0
  {
    fraction = 1; // nothing has been transmitted so UTF == 0
  }
  else
  {
    // for each face we determine the unstatisfied traffic fraction and normalize it finally
    for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
    {
      //fraction += getLinkReliability(*it,layer) * (stats[layer].statisfied_requests[*it] / total_interests);
      fraction += (stats[layer].statisfied_requests[*it] / total_interests); //with linkReliability!
    }
  }

  stats[layer].unstatisfied_traffic_fraction = 1 - fraction;
}

void ForwardingStatistics::calculateLinkReliabilities(int layer)
{
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
  {
    if(stats[layer].unstatisfied_requests[*it] == 0)
      stats[layer].last_reliability[*it] = 1.0;
    else
      stats[layer].last_reliability[*it] =
          (double)stats[layer].statisfied_requests[*it] / ((double)(stats[layer].unstatisfied_requests[*it] + stats[layer].statisfied_requests[*it]));

    NS_LOG_DEBUG("Reliabilty for Face(" << *it << ")=" << stats[layer].last_reliability[*it] << "      in total "
        << stats[layer].unstatisfied_requests[*it] + stats[layer].statisfied_requests[*it] << " interest forwarded");
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

double ForwardingStatistics::getLinkReliability(int face_id, int layer)
{
  return stats[layer].last_reliability[face_id];
}

double ForwardingStatistics::getGoodput(int face_id, int layer)
{
  return stats[layer].last_goodput[face_id];
}

std::vector<int> ForwardingStatistics::getReliableFaces(int layer, double threshold)
{
  std::vector<int> faces;

  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
  {
    if(*it == DROP_FACE_ID)
      continue;

    if(getLinkReliability(*it, layer) >= threshold)
      faces.push_back (*it);
  }

  return faces;
}

std::vector<int> ForwardingStatistics::getUnreliableFaces(int layer, double threshold)
{
  std::vector<int> faces;

  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each face
  {
    if(*it == DROP_FACE_ID)
      continue;

    if(getLinkReliability(*it, layer) < threshold)
      faces.push_back (*it);
  }

  return faces;
}

double ForwardingStatistics::getNormalizedLinkReliability(int face_id, int layer, std::vector<int> set_of_faces)
{
  double sum = getSumOfReliabilies(set_of_faces, layer);

  if(sum == 0)
    return 0;

  return getLinkReliability (face_id,layer) / sum;
}

double ForwardingStatistics::getSumOfReliabilies(std::vector<int> set_of_faces, int layer)
{
  //sum up reliabilities
  double sum = 0.0;
  for(std::vector<int>::iterator it = set_of_faces.begin(); it != set_of_faces.end(); ++it)
  {
    sum += getLinkReliability (*it, layer);
  }
  return sum;
}

double ForwardingStatistics::getSumOfUnreliabilies(std::vector<int> set_of_faces, int layer)
{
  //sum up reliabilities
  double sum = 0.0;
  for(std::vector<int>::iterator it = set_of_faces.begin(); it != set_of_faces.end(); ++it)
  {
    sum += 1 - getLinkReliability (*it, layer);
  }
  return sum;
}

double ForwardingStatistics::getActualForwardingProbability(int face_id, int layer)
{
  return stats[layer].last_actual_forwarding_probs[face_id];
}

void ForwardingStatistics::calculateActualForwardingProbabilities (int layer)
{
  double sum = stats[layer].total_forwarded_requests;

  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    if(sum == 0)
      stats[layer].last_actual_forwarding_probs[*it] = 0;
    else
      stats[layer].last_actual_forwarding_probs[*it] =
        (stats[layer].unstatisfied_requests[*it] + stats[layer].statisfied_requests[*it]) / sum;
  }
}

double ForwardingStatistics::getForwardedInterests(int face_id, int layer)
{
  return getActualForwardingProbability (face_id,layer) * getTotalForwardedInterests (layer);
}

double ForwardingStatistics::calculateUnstatisfiedTrafficFractionOfUnreliableFaces(int layer)
{
  if(stats[layer].total_forwarded_requests == 0)
    return 0;

  std::vector<int> r_faces = getReliableFaces(layer, RELIABILITY_THRESHOLD);
  r_faces.push_back (DROP_FACE_ID);
  std::vector<int> u_faces = getUnreliableFaces(layer,RELIABILITY_THRESHOLD);
  double utf = 1.0;

  for(std::vector<int>::iterator it = u_faces.begin(); it != u_faces.end(); ++it)
  {
    utf -= ( (double) (stats[layer].statisfied_requests[*it])) / getTotalForwardedInterests(layer);
  }

  for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it)
  {
    utf -= ( (double) (stats[layer].statisfied_requests[*it] + stats[layer].unstatisfied_requests[*it]) ) / getTotalForwardedInterests(layer);
  }

  stats[layer].unstatisfied_traffic_fraction_unreliable_faces = utf;
}

double ForwardingStatistics::calculateUnstatisfiedTrafficFractionOfReliableFaces(int layer)
{
  if(stats[layer].total_forwarded_requests == 0)
    return 0;

  std::vector<int> r_faces = getReliableFaces(layer, RELIABILITY_THRESHOLD);
  r_faces.push_back (DROP_FACE_ID);

  std::vector<int> u_faces = getUnreliableFaces(layer,RELIABILITY_THRESHOLD);
  double utf = 1.0;

  for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it)
  {
    utf -= ( (double) (stats[layer].statisfied_requests[*it])) / getTotalForwardedInterests(layer);
  }

  for(std::vector<int>::iterator it = u_faces.begin(); it != u_faces.end(); ++it)
  {
    utf -= ( (double) (stats[layer].statisfied_requests[*it] + stats[layer].unstatisfied_requests[*it]) ) / getTotalForwardedInterests(layer);
  }

  stats[layer].unstatisfied_traffic_fraction_reliable_faces = utf;
}

void ForwardingStatistics::removeFace(int faceId)
{
  //remove from vector...
  for(std::vector<int>::iterator it = faceIds.begin (); it != faceIds.end (); ++it)
  {
    if(*it == faceId)
    {
      faceIds.erase (it);
    std::sort(this->faceIds.begin(), this->faceIds.end());//order
    return;
    }
  }
  NS_LOG_UNCOND("Could not erase from statistics vector: " << faceId);
}


