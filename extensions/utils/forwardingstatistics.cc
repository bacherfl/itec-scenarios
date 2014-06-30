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

  if(unstatisfied_requests[index] == 0)
    return 1;

  return statisfied_requests[index] / (unstatisfied_requests[index] + statisfied_requests[index]);
}

double ForwardingStatistics::getGoodput(int face_id)
{
  int index = getMapIndexFromFaceID(face_id);
  double bits = goodput_bytes_received[index]*8;
  return bits / 1000 / UPDATE_INTERVALL; // kbits;
}
