#include "alwayslowestadaptationlogic.h"

NS_LOG_COMPONENT_DEFINE ("AlwaysLowestAdaptationLogic");

using namespace ns3::dashimpl;

AlwaysLowestAdaptationLogic::AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer *buf) : IAdaptationLogic(mpd, dataset_path, buf)
{
}

void AlwaysLowestAdaptationLogic::updateStatistic (Time, Time, unsigned int)
{

}

dash::mpd::IRepresentation* AlwaysLowestAdaptationLogic::getOptimalRepresentation (dash::mpd::IPeriod *period)
{
  return getLowestRepresentation(period);
}

