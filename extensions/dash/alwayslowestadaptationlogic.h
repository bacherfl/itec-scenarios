#ifndef ALWAYSLOWESTADAPTATIONLOGIC_H
#define ALWAYSLOWESTADAPTATIONLOGIC_H

#include "iadaptationlogic.h"

namespace ns3
{
  namespace dashimpl
  {
    class AlwaysLowestAdaptationLogic : public IAdaptationLogic
    {
    public:
      AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd);

      virtual std::string getNextSegmentUri ();

    };
  }
}

#endif // ALWAYSLOWESTADAPTATIONLOGIC_H
