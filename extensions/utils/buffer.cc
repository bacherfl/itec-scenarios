#include "buffer.h"

using namespace ns3::utils;

Buffer::Buffer(unsigned int maxSize)
{
  this->max_size = maxSize;
}

bool Buffer::addData (unsigned int seconds)
{
  if(cur_size + seconds < max_size)
  {
    this->cur_size += seconds;
    return true;
  }
  return false;
}

bool Buffer::consumeData (unsigned int seconds)
{
  if(cur_size - seconds >= 0)
  {
   cur_size -= seconds;
   return true;
  }
  return false;
}

bool Buffer::isEmpty ()
{
  if(cur_size == 0)
    return true;

  return false;
}

bool Buffer::isFull ()
{
  if(cur_size == max_size)
    return true;

  return false;
}

unsigned int Buffer::fillState()
{
  double percent = (100 * this->cur_size);
  percent /= this->max_size;

  return (int)(percent+0.5);
}
