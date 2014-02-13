#ifndef IADAPTATIONLOGIC_H
#define IADAPTATIONLOGIC_H

#include <string>
#include <vector>
#include "libdash/libdash.h"
#include "../utils/buffer.h"

#include <stdio.h>

namespace ns3
{
  namespace dashimpl
  {

    enum AdaptationLogicType
    {
        AlwaysBest,
        AlwaysLowest,
        RateBased
    };

    class IAdaptationLogic
    {
    public:

      //TODO
      IAdaptationLogic(dash::mpd::IMPD* mpd);

      virtual ~IAdaptationLogic(){}
      virtual std::string getNextSegmentUri() = 0;

    protected:
      dash::mpd::IMPD* mpd;
      dash::mpd::IPeriod* currentPeriod;
      dash::mpd::ISegment* currentSegment;
      std::string base_url;

      virtual dash::mpd::IRepresentation* getOptimalRepresentation(dash::mpd::IPeriod *period){return NULL;}

    private:

      virtual dash::mpd::IPeriod* getFirstPeriod();

      virtual dash::mpd::IRepresentation* getBestRepresentation(dash::mpd::IPeriod* period);
      virtual dash::mpd::IRepresentation* getLowestRepresentation(dash::mpd::IPeriod* period);

    };
  }
}
#endif // IADAPTATIONLOGIC_H
