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
      Segment(std::string uri, unsigned int size, unsigned int duration, unsigned int level = 0);

      unsigned int getSize();
      std::string getUri();
      unsigned int getDuration();
      unsigned int getLevel ();

    private:

      std::string uri;
      unsigned int size;
      unsigned int duration;
      unsigned int level;

    };
  }
}

#endif // SEGMENT_H
