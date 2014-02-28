#include "segment.h"

NS_LOG_COMPONENT_DEFINE ("Segment");

using namespace ns3::utils;

Segment::Segment(std::string uri, unsigned int size, unsigned int duration, unsigned int level)
{
  this->uri = uri;
  this->size = size;
  this->duration = duration;
  this->level = level;
}

std::string Segment::getUri ()
{
  return uri;
}

unsigned int Segment::getSize ()
{
  return size;
}


unsigned int Segment::getDuration ()
{
  return duration;
}

unsigned int Segment::getLevel ()
{
  return level;
}
