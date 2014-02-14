#include "alwayslowestadaptationlogic.h"

using namespace ns3::dashimpl;

AlwaysLowestAdaptationLogic::AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd) : IAdaptationLogic(mpd)
{
}

std::string AlwaysLowestAdaptationLogic::getNextSegmentUri ()
{
  //dash::mpd::IRepresentation* rep = this->getLowestRepresentation();
  //if(this->currentSegment == NULL)
  return NULL;
}

