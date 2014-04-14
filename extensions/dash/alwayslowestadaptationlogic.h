#ifndef ALWAYSLOWESTADAPTATIONLOGIC_H
#define ALWAYSLOWESTADAPTATIONLOGIC_H

#include "iadaptationlogic.h"

#include <stdio.h>

namespace ns3
{
  namespace dashimpl
  {

    /*!
     * \brief The AlwaysLowestAdaptationLogic. Returns always the lowest Represntation for a Period in a MPD.
     */
    class AlwaysLowestAdaptationLogic : public IAdaptationLogic
    {
    public:
      AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer* buf);
      virtual void updateStatistic(Time start, Time stop, unsigned int segment_size);

    protected:
      virtual dash::mpd::IRepresentation* getOptimalRepresentation (dash::mpd::IPeriod *period);

    };
  }
}

#endif // ALWAYSLOWESTADAPTATIONLOGIC_H
