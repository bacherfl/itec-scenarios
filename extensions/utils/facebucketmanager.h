#ifndef FACEBUCKETMANAGER_H
#define FACEBUCKETMANAGER_H

#include "ns3/simple-ref-count.h"

#include "ns3/log.h"

#include "ns3-dev/ns3/point-to-point-module.h"
#include "ns3-dev/ns3/network-module.h"

#include "tokenbucket.h"
#include <string>
#include "ns3/ndn-face.h"

#define DATA_PACKET_SIZE 4200
#define INTEREST_PACKET_SIZE 50
#define TOKEN_FILL_INTERVALL 10 //ms
#define BUCKET_SIZE 25.0

namespace ns3
{
namespace ndn
{
namespace utils
{

class FaceBucketManager : public SimpleRefCount<FaceBucketManager>
{
public:
  FaceBucketManager(Ptr<Face> face);
  ~FaceBucketManager();

  bool addNewBucket(std::string content_prefix);

  bool tryForwardInterest(std::string prefix);

protected:
  Ptr<Face> face;

  /* map for storing forwarding stats for all faces */
  typedef std::map
    < std::string, /*face ID*/
      Ptr<TokenBucket> /*face bucket manager*/
    > BucketMap;

  BucketMap bMap;

  void newToken();

  uint64_t getPhysicalBitrate(Ptr<Face> face);
  double tokenGenRate;

  EventId newTokenEvent;

  std::vector<std::string> getAllNonFullBuckets();
};

}
}
}
#endif // FACEBUCKETMANAGER_H
