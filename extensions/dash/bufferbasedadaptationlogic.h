#ifndef BUFFERBASEDADAPTATIONLOGIC_H
#define BUFFERBASEDADAPTATIONLOGIC_H

#include "iadaptationlogic.h"

namespace ns3
{
  namespace dashimpl
  {
    class BufferBasedAdaptationLogic : public IAdaptationLogic
    {
    public:
      BufferBasedAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer* buf);
      virtual void updateStatistic(Time start, Time stop, unsigned int segment_size);

    protected:
      virtual dash::mpd::IRepresentation* getOptimalRepresentation (dash::mpd::IPeriod *period);
    };
  }
}

#endif // BUFFERBASEDADAPTATIONLOGIC_H
