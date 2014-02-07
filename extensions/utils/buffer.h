#ifndef BUFFER_H
#define BUFFER_H

class Buffer
{
public:
  Buffer(int maxSize);

  bool consumeData(int seconds);
  bool addData(int seconds);

  bool isEmpty();
  bool isFull();

  unsigned int max_size;
  unsigned int cur_size; // in seconds
};

#endif // BUFFER_H
