#ifndef LAYEREDADAPTATIONLOGIC_H
#define LAYEREDADAPTATIONLOGIC_H

#include "../dash/iadaptationlogic.h"
#include "layeredbuffer.h"

#include <vector>


namespace ns3
{
  namespace dashimpl
  {
    class LayeredAdaptationLogic : public IAdaptationLogic
    {
      public:
        LayeredAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, Ptr<ns3::player::LayeredBuffer> buf);
        virtual std::vector<Ptr<utils::Segment> > getNextSegments();

        virtual void segmentRetrieved(Time start, Time stop,
                                      unsigned int segment_number, unsigned int segment_level, unsigned int segment_size);

        virtual void segmentFailed(unsigned int segment_number, unsigned int segment_level);

        virtual void segmentConsumed(unsigned int segment_number);

      protected:

        enum AdaptationPhase
        {
          Steady = 0,
          Growing = 1,
          Upswitching = 2
        };

        Ptr<ns3::player::LayeredBuffer> buf;
        virtual dash::mpd::IRepresentation* getOptimalRepresentation (dash::mpd::IPeriod *period);

        unsigned int desired_buffer_size(int i, int i_curr);
        unsigned int getNextNeededSegmentNumber(int level);

        unsigned int last_consumed_segment_number;


        std::vector<int> averageBandwidth;

        double alpha; //
        int gamma; //BUFFER_MIN_SIZE
        int segments_for_growing;
        int segments_for_upswitching;

        AdaptationPhase lastPhase;
        AdaptationPhase allowedPhase;
    };
  }
}

#endif // LAYEREDADAPTATIONLOGIC_H
