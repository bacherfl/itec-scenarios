#ifndef ITRACER_H
#define ITRACER_H

#include "ns3-dev/ns3/ndn-forwarding-strategy.h"
#include "ns3-dev/ns3/ndn-face-container.h"
#include "ns3-dev/ns3/ndn-face.h"
#include "ns3-dev/ns3/ndn-data.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-pit.h"
#include "ns3-dev/ns3/callback.h"

#include <stdio.h>

#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>


namespace ns3 {
namespace ndn {
namespace utils {

class ITracer
{

public:
  ITracer(ForwardingStrategy* strategy, Time avgPeriod);

  virtual ~ITracer () {}

  void addFace(Ptr<const Face> face);
  void removeFace(Ptr<const Face> face);
  void setAvgPeriod (const Time &avgPeriod);

  unsigned int getAvgInTrafficBits(Ptr<const Face> face);
  unsigned int getAvgOutTrafficBits(Ptr<const Face> face);
  unsigned int getAvgTotalTrafficBits(Ptr<const Face> face);

  //todo implement public methods e.g. getAvgSpeed..etc

protected:
  void Connect ();

  void copyDataForAvgCalculation();

  virtual void OutInterests  (Ptr<const Interest>, Ptr<const Face>) = 0;
  virtual void InInterests   (Ptr<const Interest>, Ptr<const Face>) = 0;
  virtual void DropInterests (Ptr<const Interest>, Ptr<const Face>) = 0;
  virtual void OutNacks  (Ptr<const Interest>, Ptr<const Face>) = 0;
  virtual void InNacks   (Ptr<const Interest>, Ptr<const Face>) = 0;
  virtual void DropNacks (Ptr<const Interest>, Ptr<const Face>) = 0;
  virtual void OutData  (Ptr<const Data>, bool fromCache, Ptr<const Face>) = 0;
  virtual void InData   (Ptr<const Data>, Ptr<const Face>) = 0;
  virtual void DropData (Ptr<const Data>, Ptr<const Face>) = 0;
  virtual void SatisfiedInterests (Ptr<const pit::Entry>) = 0;
  virtual void TimedOutInterests (Ptr<const pit::Entry>) = 0;

  struct Stats
  {
    Stats(){Reset();}

    void Reset ()
    {
      m_inInterestsCount   = 0;
      m_outInterestsCount  = 0;
      m_dropInterestsCount = 0;
      m_inNacksCount       = 0;
      m_outNacksCount      = 0;
      m_dropNacksCount     = 0;
      m_inDataCount        = 0;
      m_outDataCount       = 0;
      m_dropDataCount      = 0;
      m_satisfiedInterestsCount = 0;
      m_timedOutInterestsCount = 0;
      m_outSatisfiedInterestsCount = 0;
      m_outTimedOutInterestsCount = 0;

      /*Size is stored in Bytes*/
      m_inInterestsSize   = 0;
      m_outInterestsSize  = 0;
      m_dropInterestsSize = 0;
      m_inNacksSize       = 0;
      m_outNacksSize      = 0;
      m_dropNacksSize     = 0;
      m_inDataSize        = 0;
      m_outDataSize       = 0;
      m_dropDataSize      = 0;
      m_satisfiedInterestsSize = 0;
      m_timedOutInterestsSize = 0;
      m_outSatisfiedInterestsSize = 0;
      m_outTimedOutInterestsSize = 0;
    }

    Stats operator=(const Stats& other)
    {
      m_inInterestsCount   = other.m_inInterestsCount;
      m_outInterestsCount  = other.m_outInterestsCount;
      m_dropInterestsCount = other.m_dropInterestsCount;
      m_inNacksCount       = other.m_inNacksCount;
      m_outNacksCount      = other.m_outNacksCount;
      m_dropNacksCount     = other.m_dropNacksCount;
      m_inDataCount        = other.m_inDataCount;
      m_outDataCount       = other.m_outDataCount;
      m_dropDataCount      = other.m_dropDataCount;
      m_satisfiedInterestsCount = other.m_satisfiedInterestsCount;
      m_timedOutInterestsCount = other.m_timedOutInterestsCount;
      m_outSatisfiedInterestsCount = other.m_outSatisfiedInterestsCount;
      m_outTimedOutInterestsCount = other.m_outTimedOutInterestsCount;

      m_inInterestsSize   = other.m_inInterestsSize;
      m_outInterestsSize  = other.m_outInterestsSize;
      m_dropInterestsSize = other.m_dropInterestsSize;
      m_inNacksSize       = other.m_inNacksSize;
      m_outNacksSize      = other.m_outNacksSize;
      m_dropNacksSize     = other.m_dropNacksSize;
      m_inDataSize        = other.m_inDataSize;
      m_outDataSize       = other.m_outDataSize;
      m_dropDataSize      = other.m_dropDataSize;
      m_satisfiedInterestsSize = other.m_satisfiedInterestsSize;
      m_timedOutInterestsSize = other.m_timedOutInterestsSize;
      m_outSatisfiedInterestsSize = other.m_outSatisfiedInterestsSize;
      m_outTimedOutInterestsSize = other.m_outTimedOutInterestsSize;

      return *this;
    }

    int m_inInterestsCount, m_inInterestsSize;
    int m_outInterestsCount, m_outInterestsSize;
    int m_dropInterestsCount, m_dropInterestsSize;
    int m_inNacksCount, m_inNacksSize;
    int m_outNacksCount, m_outNacksSize;
    int m_dropNacksCount, m_dropNacksSize;
    int m_inDataCount, m_inDataSize;
    int m_outDataCount, m_outDataSize;
    int m_dropDataCount, m_dropDataSize;
    int m_satisfiedInterestsCount, m_satisfiedInterestsSize;
    int m_timedOutInterestsCount, m_timedOutInterestsSize;
    int m_outSatisfiedInterestsCount, m_outSatisfiedInterestsSize;
    int m_outTimedOutInterestsCount, m_outTimedOutInterestsSize;
  };

  ForwardingStrategy *strategy;
  Time period;
  typedef boost::unordered_map<int, Stats> StatsMap;
  StatsMap faceStats; /*used for gathering facestats*/
  StatsMap measuredStats; /*after period times out, faceStats are copied into this struct. measuredStats is used for avgCalculation*/
  EventId avgEvent;

};

} // utils
} // namespace ndn
} // namespace ns3

#endif // ITRACER_H
