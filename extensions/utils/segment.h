#ifndef SEGMENT_H
#define SEGMENT_H

#include <string>
#include <sstream>

#include "ns3-dev/ns3/log.h"

namespace ns3
{
  namespace utils
  {
    class Segment
    {
    public:
      Segment(std::string uri, unsigned int size,
              unsigned int duration, unsigned int avgLevelBitrate,
              unsigned int level, unsigned int segmentNumber);

      unsigned int getSize();
      std::string getUri();
      unsigned int getDuration();
      unsigned int getAvgLvlBitrate();
      unsigned int getLevel ();
      unsigned int getSegmentNumber ();

      std::string toString();

    private:

      std::string uri;
      unsigned int size;
      unsigned int duration;
      unsigned int avgLevelBitrate;
      unsigned int level;
      unsigned int segmentNumber;

      std::string convertInt(unsigned int number);

    };
  }
}

#endif // SEGMENT_H
