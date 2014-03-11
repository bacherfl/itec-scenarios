#include "segment.h"

NS_LOG_COMPONENT_DEFINE ("Segment");

using namespace ns3::utils;

Segment::Segment(std::string uri, unsigned int size, unsigned int duration, unsigned int avgLevelBitrate, unsigned int level)
{
  this->uri = uri;
  this->size = size;
  this->duration = duration;
  this->avgLevelBitrate = avgLevelBitrate;
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

std::string Segment::toString ()
{
  std::string ret("");
  ret.append ("SegUri: ").append (uri);
  ret.append (" SegSize: ").append(convertInt (size));
  ret.append (" SegDuration: ").append(convertInt (duration));
  ret.append ("SegAvgLvlBitrate: ").append (convertInt (avgLevelBitrate));
  ret.append (" SegLevel: ").append(convertInt (level));

  return ret;
}

std::string Segment::convertInt(unsigned int number)
{
   std::stringstream ss;
   ss << number;
   return ss.str();
}

