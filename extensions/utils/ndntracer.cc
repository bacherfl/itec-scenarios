#include "ndntracer.h"

using namespace ns3::ndn::utils;

NDNTracer::NDNTracer(ForwardingStrategy* strategy, Time averagingPeriod) : ITracer(strategy, averagingPeriod)
{
  Stats a,b;
  b.m_dropInterestsCount = 10;

  a = b;
  a.m_dropInterestsCount++;

  fprintf(stderr, "a.m_dropInterestsCount = %d\n", a.m_dropInterestsCount);
  fprintf(stderr, "b.m_dropInterestsCount = %d\n", b.m_dropInterestsCount);
}

void NDNTracer::OutInterests  (Ptr<const Interest> interest, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_outInterestsCount ++;

  if(interest->GetWire ())
    faceStats[face->GetId ()].m_outInterestsSize += interest->GetWire ()->GetSize ();
}

void NDNTracer::InInterests   (Ptr<const Interest> interest, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_inInterestsCount ++;

  if(interest->GetWire ())
    faceStats[face->GetId ()].m_inInterestsSize += interest->GetWire ()->GetSize ();
}

void NDNTracer::DropInterests (Ptr<const Interest> interest, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_dropInterestsCount ++;

  if(interest->GetWire ())
    faceStats[face->GetId ()].m_dropInterestsSize += interest->GetWire ()->GetSize();
}

void NDNTracer::OutNacks  (Ptr<const Interest> interest, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_outNacksCount ++;

  if(interest->GetWire ())
  faceStats[face->GetId ()].m_outNacksSize += interest->GetWire ()->GetSize ();
}

void NDNTracer::InNacks   (Ptr<const Interest> interest, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_inNacksCount ++;

  if(interest->GetWire ())
  faceStats[face->GetId ()].m_inNacksSize += interest->GetWire ()->GetSize ();
}

void NDNTracer::DropNacks (Ptr<const Interest> interest, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_dropNacksCount ++;

  if(interest->GetWire ())
  faceStats[face->GetId ()].m_dropNacksSize += interest->GetWire ()->GetSize ();
}

void NDNTracer::OutData  (Ptr<const Data> data, bool fromCache, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_outDataCount ++;

  if(data->GetWire ())
  faceStats[face->GetId ()].m_outDataSize += data->GetWire ()->GetSize ();
}

void NDNTracer::InData   (Ptr<const Data> data, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_inDataCount ++;

  if(data->GetWire ())
  faceStats[face->GetId ()].m_inDataSize += data->GetWire ()->GetSize ();
}

void NDNTracer::DropData (Ptr<const Data> data, Ptr<const Face> face)
{
  faceStats[face->GetId ()].m_dropDataCount ++;

  if(data->GetWire ())
  faceStats[face->GetId ()].m_dropDataSize += data->GetWire ()->GetSize ();
}

void NDNTracer::SatisfiedInterests (Ptr<const pit::Entry>)
{
  //TODO
  //fprintf(stderr, "EVENT SatisfiedInterests TODO\n");
}

void NDNTracer::TimedOutInterests (Ptr<const pit::Entry>)
{
  //TODO
  //fprintf(stderr, "EVENT TimedOutInterests TODO\n");
}

