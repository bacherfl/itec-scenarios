#include "itracer.h"

using namespace  ns3::ndn::utils;

ITracer::ITracer(ForwardingStrategy* strategy, Time avgPeriod)
{
  this->strategy = strategy;
  Connect();
  setAvgPeriod (avgPeriod);
}


void ITracer::addFace(Ptr<const Face> face)
{
  //fprintf(stderr, "Added face %d to tracing\n", face->GetId ());
  faceStats[face->GetId ()] = Stats();
}

void ITracer::removeFace(Ptr<const Face> face)
{
  faceStats.erase(face->GetId ());
}

void ITracer::setAvgPeriod (const Time &avgPeriod)
{
  this->period = avgPeriod;
  avgEvent.Cancel ();
  avgEvent = Simulator::Schedule (period, &ITracer::copyDataForAvgCalculation, this);
}

void ITracer::copyDataForAvgCalculation ()
{
  measuredStats.clear();

  BOOST_FOREACH(StatsMap::value_type &i, faceStats)
  {
    measuredStats.insert(i);
    i.second.Reset();
  }
  avgEvent = Simulator::Schedule (period, &ITracer::copyDataForAvgCalculation, this);
}

unsigned int ITracer::getAvgInTrafficBits(Ptr<const Face> face)
{
  Stats s = measuredStats[face->GetId ()];

  int bits = (s.m_inDataSize + s.m_inInterestsSize + s.m_inNacksSize) * 8;
  return ( bits / period.GetSeconds());
}

unsigned int ITracer::getAvgOutTrafficBits(Ptr<const Face> face)
{
  Stats s = measuredStats[face->GetId ()];

  int bits = (s.m_outDataSize + s.m_outInterestsSize + s.m_outNacksSize) *8;
  return ( bits / period.GetSeconds());
}

unsigned int ITracer::getAvgTotalTrafficBits (Ptr<const Face> face)
{
  return getAvgInTrafficBits (face) + getAvgOutTrafficBits (face);
}


void ITracer::Connect ()
{
  strategy->TraceConnectWithoutContext ("OutInterests",  MakeCallback (&ITracer::OutInterests, this));
  strategy->TraceConnectWithoutContext ("InInterests",   MakeCallback (&ITracer::InInterests, this));
  strategy->TraceConnectWithoutContext ("DropInterests", MakeCallback (&ITracer::DropInterests, this));

  strategy->TraceConnectWithoutContext ("OutData",  MakeCallback (&ITracer::OutData, this));
  strategy->TraceConnectWithoutContext ("InData",   MakeCallback (&ITracer::InData, this));
  strategy->TraceConnectWithoutContext ("DropData", MakeCallback (&ITracer::DropData, this));

  strategy->TraceConnectWithoutContext ("OutNacks",  MakeCallback (&ITracer::OutNacks, this));
  strategy->TraceConnectWithoutContext ("InNacks",   MakeCallback (&ITracer::InNacks, this));
  strategy->TraceConnectWithoutContext ("DropNacks", MakeCallback (&ITracer::DropNacks, this));

  strategy->TraceConnectWithoutContext ("SatisfiedInterests", MakeCallback (&ITracer::SatisfiedInterests, this));
  strategy->TraceConnectWithoutContext ("TimedOutInterests", MakeCallback (&ITracer::TimedOutInterests, this));
}
