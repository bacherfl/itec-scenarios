#ifndef SEGMENT_H
#define SEGMENT_H

#include <string>

#include "ns3-dev/ns3/log.h"

namespace ns3
{
  namespace utils
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
