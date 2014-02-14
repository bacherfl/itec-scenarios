#ifndef ALWAYSLOWESTADAPTATIONLOGIC_H
#define ALWAYSLOWESTADAPTATIONLOGIC_H

#include "iadaptationlogic.h"

#include <stdio.h>

namespace ns3
{
  namespace dashimpl
  {
    class AlwaysLowestAdaptationLogic : public IAdaptationLogic
    {
    public:
      AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path);

      virtual Segment* getNextSegmentUri ();

    };
  }
}

#endif // ALWAYSLOWESTADAPTATIONLOGIC_H
