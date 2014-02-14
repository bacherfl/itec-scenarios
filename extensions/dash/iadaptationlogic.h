#ifndef IADAPTATIONLOGIC_H
#define IADAPTATIONLOGIC_H

#include <string>
#include <vector>
#include "libdash/libdash.h"

#include "../utils/buffer.h"
#include "segment.h"

#include <limits.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

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
      IAdaptationLogic(dash::mpd::IMPD* mpd, std::string dataset_path);

      virtual ~IAdaptationLogic(){}
      virtual Segment* getNextSegmentUri() = 0;

    protected:
      dash::mpd::IMPD* mpd;
      std::string dataset_path;
      dash::mpd::IPeriod* currentPeriod;
      unsigned int currentSegmentNr;
      std::string base_url;

      virtual dash::mpd::IRepresentation* getOptimalRepresentation(dash::mpd::IPeriod *period){return NULL;}

      virtual dash::mpd::IPeriod* getFirstPeriod();

      virtual dash::mpd::IRepresentation* getBestRepresentation(dash::mpd::IPeriod* period);
      virtual dash::mpd::IRepresentation* getLowestRepresentation(dash::mpd::IPeriod* period);
      virtual unsigned int getFileSize(std::string filename);

    };
  }
}
#endif // IADAPTATIONLOGIC_H
