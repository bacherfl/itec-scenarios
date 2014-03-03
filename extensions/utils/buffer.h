#ifndef MYBUFFER_H
#define MYBUFFER_H

#include <pthread.h>

namespace ns3
{
  namespace utils
  {
    class Buffer
    {
    public:
      Buffer(unsigned int maxSize);

      bool consumeData(unsigned int seconds);
      bool addData(unsigned int seconds);

      bool isEmpty();
      bool isFull();

      double fillPercentage();
      unsigned int maxBufferSeconds();
      unsigned int bufferedSeconds();

    private:
      unsigned int max_size;
      unsigned int cur_size; // in seconds
      pthread_mutex_t mutex;
    };
  }
}

#endif // BUFFER_H
