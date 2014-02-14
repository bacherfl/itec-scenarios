#include "segment.h"

using namespace ns3::dashimpl;

Segment::Segment(std::string uri, unsigned int size)
{
  this->uri = uri;
  this->size = size;
}

std::string Segment::getUri ()
{
  return uri;
}

unsigned int Segment::getSize ()
{
  return size;
}
