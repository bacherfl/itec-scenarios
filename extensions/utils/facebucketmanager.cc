#include "facebucketmanager.h"

using namespace ns3::ndn::utils;

NS_LOG_COMPONENT_DEFINE ("FaceBucketManager");

FaceBucketManager::FaceBucketManager(Ptr<Face> face)
{

  //fprintf(stderr, "FaceBucketManager::FaceBucketManager()\n");

  this->face = face;

  //fprintf(stderr, "faceBitrate = %d", getPhysicalBitrate (face));

  double packets_per_sec = getPhysicalBitrate (face) / 8 ;
  packets_per_sec /= (DATA_PACKET_SIZE + INTEREST_PACKET_SIZE);
  tokenGenRate = packets_per_sec / 1000; // tokens per ms
  tokenGenRate *= TOKEN_FILL_INTERVALL; // tokens per intervall

  //fprintf(stderr, "packets_per_sec %f\n", packets_per_sec );
  //fprintf(stderr, "tokenGenRate %f\n", tokenGenRate );

  this->newTokenEvent = Simulator::Schedule(Seconds(0), &FaceBucketManager::newToken, this);
}

FaceBucketManager::~FaceBucketManager ()
{
  Simulator::Cancel (this->newTokenEvent);
}

bool FaceBucketManager::addNewBucket(std::string content_prefix)
{
  bMap[content_prefix] = Create<TokenBucket>(BUCKET_SIZE); // for now we give all tokenbuckets a const size we should adapt this later
}

void  FaceBucketManager::newToken()
{
  double rest = tokenGenRate;

  std::vector<std::string> indizes = getAllNonFullBuckets();

  while(indizes.size () != 0 && rest != 0)
  {
    double tokens = rest;
    rest = 0;

    for(std::vector<std::string>::iterator it = indizes.begin (); it != indizes.end (); ++it)
    {
      rest += bMap[*it]->addTokens(tokens / (double) indizes.size ());
      //fprintf(stderr, "added %f tokens to bucket %s\n", tokens / (double) indizes.size (), (*it).c_str());
    }

    indizes = getAllNonFullBuckets ();
  }

  this->newTokenEvent = Simulator::Schedule(MilliSeconds(TOKEN_FILL_INTERVALL), &FaceBucketManager::newToken, this);
}


std::vector<std::string> FaceBucketManager::getAllNonFullBuckets()
{
  std::vector<std::string> vec;

  //fprintf(stderr, "bMap.size = %d; refcount = %d\n", bMap.size (), this->GetReferenceCount ());

  for(BucketMap::iterator it = bMap.begin (); it != bMap.end (); ++it)
  {
    if(!it->second->isFull())
      vec.push_back(it->first);
  }

  return vec;
}

uint64_t FaceBucketManager::getPhysicalBitrate(Ptr<Face> face)
{
  // Get Device Bitrate of that face (make sure to call face->Getid()
  //fprintf(stderr, "FaceID = %d\n", face->GetId());
  //fprintf(stderr, "NodeID = %d\n", face->GetNode ()->GetId ());
  Ptr<PointToPointNetDevice> nd1 = face->GetNode()->GetDevice(face->GetId())->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  nd1->GetAttribute("DataRate", dv);
  DataRate d = dv.Get();
  //fprintf(stderr, "bitrate on face = %llu\n", d.GetBitRate());
  return d.GetBitRate();
}

bool FaceBucketManager::tryForwardInterest(std::string prefix)
{
  return bMap[prefix]->tryConsumeToken();
}
