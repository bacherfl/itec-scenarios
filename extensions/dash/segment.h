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
      Segment(std::string uri, unsigned int size);

      unsigned int getSize();
      std::string getUri();

    private:

      std::string uri;
      unsigned int size;

    };
  }
}

#endif // SEGMENT_H
