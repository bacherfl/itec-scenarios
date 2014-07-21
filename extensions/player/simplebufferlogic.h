#ifndef SIMPLEBUFFERLOGIC_H
#define SIMPLEBUFFERLOGIC_H

#include "../dash/iadaptationlogic.h"
#include "layeredbuffer.h"

#define DESIRED_BUFFER_SIZE_IN_SEGMENTS 15
#define OFFSET 3

namespace ns3
{
namespace player
{

class SimpleBufferLogic : public dashimpl::IAdaptationLogic
{
public:
  SimpleBufferLogic(dash::mpd::IMPD *mpd, std::string dataset_path, Ptr<player::LayeredBuffer> buf);

  virtual dash::mpd::IRepresentation* getOptimalRepresentation (dash::mpd::IPeriod *period);

  virtual std::vector<Ptr<utils::Segment> > getNextSegments();

  virtual void segmentRetrieved(Time start, Time stop,
                                unsigned int segment_number, unsigned int segment_level, unsigned int segment_size);

  virtual void segmentFailed(unsigned int segment_number, unsigned int segment_level);

  virtual void segmentConsumed(unsigned int segment_number, unsigned int segment_level);

  unsigned int getNextNeededSegmentNumber(int level);

protected:
  Ptr<ns3::player::LayeredBuffer> buf;
  unsigned int last_consumed_segment_number;
  unsigned int max_level;

};
}
}
#endif // SIMPLEBUFFERLOGIC_H
