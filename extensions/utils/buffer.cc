#include "buffer.h"

using namespace ns3::utils;

Buffer::Buffer(unsigned int maxSize)
{
   pthread_mutex_init(&mutex, NULL);
  this->max_size = maxSize;
  this->cur_size = 0;
}

bool Buffer::addData (unsigned int seconds)
{
  bool result = false;

  pthread_mutex_lock (&mutex);

  if(cur_size + seconds < max_size)
  {
    cur_size += seconds;
    result = true;
  }

  pthread_mutex_unlock(&mutex);
  return result;
}

bool Buffer::consumeData (unsigned int seconds)
{
  bool result = false;

  pthread_mutex_lock (&mutex);

  if(cur_size >= seconds)
  {
   cur_size -= seconds;
   result = true;
  }

  pthread_mutex_unlock(&mutex);
  return result;
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

unsigned int Buffer::fillPercentage()
{
  double percent = (100 * cur_size);
  percent /= this->max_size;

  return (int)(percent+0.5);
}

unsigned int Buffer::maxBufferSeconds()
{
  return max_size;
}

unsigned int Buffer::bufferedSeconds()
{
  return cur_size;
}
