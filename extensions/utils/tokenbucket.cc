#include "tokenbucket.h"

using namespace ns3::ndn::utils;

TokenBucket::TokenBucket(double maxTokens)
{
  this->maxTokens = maxTokens;

  // put some tokens into the bucket at beginning.
  this->tokens = maxTokens / 5;
}

double TokenBucket::addTokens(double tokens)
{
  double ret = 0.0;
  this->tokens += tokens;

  if(this->tokens > maxTokens)
  {
    ret = this->tokens - maxTokens;
    this->tokens = maxTokens;
  }

  return ret;
}

bool TokenBucket::tryConsumeToken()
{

  if(tokens >= 1)
  {
    tokens-=1;
    return true;
  }

  return false;
}

bool TokenBucket::isFull ()
{
  if(tokens >= maxTokens)
    return true;

  return false;
}

void TokenBucket::setNewBucketSize(double maxTokens)
{
  this->maxTokens = maxTokens;

  if(tokens > maxTokens)
    tokens = maxTokens;
}
