#ifndef LAYEREDBUFFER_H
#define LAYEREDBUFFER_H

#include <map>
#include <vector>

#include "ns3/simple-ref-count.h"

using namespace std;


typedef map <unsigned int, vector<unsigned int> > LayerSegmentMap;

namespace ns3
{
  namespace player
  {
    class LayeredBuffer : public SimpleRefCount<LayeredBuffer>
    {
    public:
      LayeredBuffer();

      bool AddToBuffer(unsigned int segment_number, unsigned int segment_level);
      bool IsInBuffer(unsigned int segment_number, unsigned int segment_level);
      bool ConsumeFromBuffer(unsigned int segment_number, unsigned int segment_level);
      unsigned int BufferSize(unsigned int segment_level);
      unsigned int LastSegmentNumber(unsigned int segment_level);
      unsigned int GetLevelCount();

    private:
      LayerSegmentMap m_buffer;

    };
  }
}

#endif // LAYEREDBUFFER_H
