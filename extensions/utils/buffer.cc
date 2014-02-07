#include "buffer.h"

Buffer::Buffer(int maxSize)
{
  this->max_size = maxSize;
}

bool Buffer::addData (int seconds)
{
  if(cur_size + seconds < max_size)
  {
    this->cur_size += seconds;
    return true;
  }
  return false;
}

bool Buffer::consumeData (int seconds)
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
