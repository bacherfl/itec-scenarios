#include "alwayslowestadaptationlogic.h"

NS_LOG_COMPONENT_DEFINE ("AlwaysLowestAdaptationLogic");

using namespace ns3::dashimpl;

AlwaysLowestAdaptationLogic::AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer *buf)
  : IAdaptationLogic(mpd, dataset_path, buf)
{
}


void AlwaysLowestAdaptationLogic::segmentRetrieved(Time start, Time stop,
                              unsigned int segment_number, unsigned int segment_level, unsigned int segment_size)
{

}

void AlwaysLowestAdaptationLogic::segmentFailed(unsigned int segment_number, unsigned int segment_level)
{

}

void AlwaysLowestAdaptationLogic::segmentConsumed(unsigned int segment_number, unsigned int segment_level)
{

}

dash::mpd::IRepresentation* AlwaysLowestAdaptationLogic::getOptimalRepresentation (dash::mpd::IPeriod *period)
{
  return getLowestRepresentation(period);
}

