#include "bufferbasedadaptationlogic.h"

using namespace ns3::dashimpl;

BufferBasedAdaptationLogic::BufferBasedAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer *buf) : IAdaptationLogic(mpd, dataset_path, buf)
{
}

void BufferBasedAdaptationLogic::updateStatistic(Time start, Time stop, unsigned int segment_size)
{
  //TODO
}

dash::mpd::IRepresentation* BufferBasedAdaptationLogic::getOptimalRepresentation (dash::mpd::IPeriod *period)
{
  return NULL;
}
