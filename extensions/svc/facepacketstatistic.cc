#include "facepacketstatistic.h"

using namespace ns3::ndn::svc;

NS_LOG_COMPONENT_DEFINE ("FacePacketStatistic");

FacePacketStatistic::FacePacketStatistic()
{
  policy = new PacketBasedDroppingPolicy();
  stats  = new SmoothLevelStatistics();

  max_packets_per_time = 0;
}


FacePacketStatistic::~FacePacketStatistic()
{
  if(policy != NULL)
    delete policy;

  if(stats != NULL)
    delete stats;
}


void FacePacketStatistic::PrintPolicy()
{
  this->policy->Print(cout);
}


LevelStatistics* FacePacketStatistic::getStats()
{
  return stats;
}


DroppingPolicy* FacePacketStatistic::getPolicy()
{
  return policy;
}

void FacePacketStatistic::UpdatePolicy(double metric)
{
  // Refresh statistics
  this->stats->RefreshStatistics();

  //this->stats->Print(cout);

  // set metric
  this->policy->SetMetric(metric);

  // update (= feed) policy
  this->policy->Feed(*(this->stats));
  // reset stats counters
  this->stats->ResetCounters();

  //this->policy->Print(cout);
}


void FacePacketStatistic::SetMaxPacketsPerTime(unsigned int max_packets)
{
  this->max_packets_per_time = max_packets;
}


unsigned int FacePacketStatistic::GetMaxPacketsPerTime()
{
  return this->max_packets_per_time;
}


void FacePacketStatistic::IncreasePackets(unsigned int level)
{
  this->packets_per_time++;
  this->stats->IncreaseLevelCounter(level);
}


double FacePacketStatistic::GetDropProbability(unsigned int level)
{
  return this->policy->GetDroppingProbability(level);
}


void FacePacketStatistic::SetPacketsPerTime(unsigned int packets)
{
  this->packets_per_time = packets;
}


unsigned int FacePacketStatistic::GetPacketsPerTime()
{
  return this->packets_per_time;
}
