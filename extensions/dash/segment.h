#ifndef SEGMENT_H
#define SEGMENT_H

#include <string>

namespace ns3
{
  namespace dashimpl
  {
    class Segment
    {
    public:
      Segment(std::string uri, unsigned int size, unsigned int duration);

      unsigned int getSize();
      std::string getUri();
      unsigned int getDuration();

    private:

      std::string uri;
      unsigned int size;
      unsigned int duration;

    };
  }
}

#endif // SEGMENT_H
