#ifndef TOKENBUCKET_H
#define TOKENBUCKET_H

#include "ns3/simple-ref-count.h"
#include <iostream>
#include <stdio.h>

#define INITIAL_TOKENS 0.25

namespace ns3
{
namespace ndn
{
namespace utils
{

class TokenBucket : public SimpleRefCount<TokenBucket>
{
public:
  TokenBucket(double maxTokens);

  double addTokens(double tokens);
  bool tryConsumeToken();
  bool isFull();
  void setNewBucketSize(double maxTokens);

protected:

  //all tokens are in interests a X bytes
  double tokens;
  double maxTokens;

};
}
}
}
#endif // TOKENBUCKET_H
