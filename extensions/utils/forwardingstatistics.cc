#include "forwardingstatistics.h"

using namespace ns3::ndn;

ForwardingStatistics::ForwardingStatistics(std::vector<int> faceIds)
{
  this->faceIds = faceIds;
  resetStatistics();
}

// here we log all unstatisfied requests. events like timeout and NACKs
void ForwardingStatistics::logUnstatisfiedRequest(Ptr<pit::Entry> pitEntry)
{
  //fprintf(stderr, "TIMEOUT\n");
  //bytes_transmitted += pitEntry->GetInterest ()->GetWire ()->GetSize ();

  for (pit::Entry::out_container::iterator face = pitEntry->GetOutgoing ().begin ();face != pitEntry->GetOutgoing ().end (); face ++)
  {
    int index = getMapIndexFromFaceID ((*face).m_face->GetId ());
    unstatisfied_requests[index] += 1;
  }
}

//here we log all statisfied requests
void ForwardingStatistics::logStatisfiedRequest(Ptr<Face> inFace, Ptr<pit::Entry> pitEntry)
{
  int index = getMapIndexFromFaceID (inFace->GetId ());
  statisfied_requests[index] += 1;
  goodput_bytes_received[index] += 4096; // TODO get real size
}

//here we log all events were we said forward on face x but e.g. per-out-face-limits revises our decision.
void ForwardingStatistics::logExhaustedFace(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry, Ptr<Face> targetedOutFace)
{
  //fprintf(stderr, "EXHAUSTED\n");
  int index = getMapIndexFromFaceID (targetedOutFace->GetId ());
  unstatisfied_requests[index] += 1;
}

void ForwardingStatistics::resetStatistics ()
{
  last_goodput.clear ();
  last_reliability.clear();
  unstatisfied_traffic_fraction = 0;

  calculateLinkReliabilities ();
  calculateGoodput();
  calculateUnstatisfiedTrafficFraction ();

  unstatisfied_requests.clear ();
  statisfied_requests.clear ();
  goodput_bytes_received.clear ();

  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    unstatisfied_requests[*it] = 0;
    statisfied_requests[*it] = 0;
    goodput_bytes_received[*it] = 0;
  }
}

void ForwardingStatistics::calculateUnstatisfiedTrafficFraction()
{
  unstatisfied_traffic_fraction = 1.0;

  //sum up total goodput
  double total_goodput = 0.0;

  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    total_goodput += getGoodput(*it);
  }

  if(total_goodput < 0.1) // goodput == 0
  {
    unstatisfied_traffic_fraction = 0;
    return;
  }

  //fprintf(stderr, "/ total_goodput = %f\n", total_goodput);
  // for each face we determine the unstatisfied traffic fraction and normalize it finally
  double fraction = 0;
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    fraction += getLinkReliability(*it) * (getGoodput (*it) / total_goodput);
  }

  unstatisfied_traffic_fraction -= fraction;
}

void ForwardingStatistics::calculateLinkReliabilities()
{
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    if(unstatisfied_requests[*it] == 0)
      last_reliability[*it] = 1.0;
    else
      last_reliability[*it] = (double)statisfied_requests[*it] / (double)(unstatisfied_requests[*it] + statisfied_requests[*it]);
  }
}

void ForwardingStatistics::calculateGoodput()
{
  for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
  {
    double bits = goodput_bytes_received[*it]*8;
    last_goodput[*it] = bits / 1000 / UPDATE_INTERVALL; // kbits;
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

double ForwardingStatistics::getLinkReliability(int face_id)
{
  int index = getMapIndexFromFaceID (face_id);
  return last_reliability[index];
}

double ForwardingStatistics::getGoodput(int face_id)
{
  int index = getMapIndexFromFaceID (face_id);
  return last_goodput[index];
}
