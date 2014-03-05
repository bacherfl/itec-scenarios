#include "itracer.h"

using namespace  ns3::ndn::utils;

ITracer::ITracer(ForwardingStrategy* strategy)
{
  this->strategy = strategy;
  Connect();
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
