#include "segment.h"

using namespace ns3::dashimpl;

Segment::Segment(std::string uri, unsigned int size, unsigned int duration)
{
  this->uri = uri;
  this->size = size;
  this->duration = duration;
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
