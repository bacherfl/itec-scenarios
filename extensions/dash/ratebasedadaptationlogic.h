#ifndef RATEBASEDADAPTATIONLOGIC_H
#define RATEBASEDADAPTATIONLOGIC_H

#include "iadaptationlogic.h"

#define SEGMENT_HISTORY 10

namespace ns3
{
  namespace dashimpl
  {
    /*!
     * \brief The RateBasedAdaptationLogic is an AdaptationLogic that considers the average download speed of SEGMENT_HISTORY segments and the buffer fillstate. \see updateStatistic() for the forumla.
     */
    class RateBasedAdaptationLogic : public IAdaptationLogic
    {
    public:
      RateBasedAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer *buf);
      virtual void segmentRetrieved(Time start, Time stop,
                                    unsigned int segment_number, unsigned int segment_level, unsigned int segment_size);

      virtual void segmentFailed(unsigned int segment_number, unsigned int segment_level);
      virtual void segmentConsumed(unsigned int segment_number, unsigned int segment_level);

    protected:
      dash::mpd::IRepresentation* getOptimalRepresentation(dash::mpd::IPeriod *period);

      unsigned int getAvgDLS();

      int64_t* avg_dl_rates;
      int64_t index;

    };
    }
}

#endif // RATEBASEDADAPTATIONLOGIC_H
