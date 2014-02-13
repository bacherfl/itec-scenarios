#ifndef MYBUFFER_H
#define MYBUFFER_H

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

      unsigned int fillState();

    private:
      unsigned int max_size;
      unsigned int cur_size; // in seconds
    };
  }
}

#endif // BUFFER_H
