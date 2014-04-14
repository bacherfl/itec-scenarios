#include "facepacketstatistic.h"

using namespace ns3::ndn::svc;

NS_LOG_COMPONENT_DEFINE ("FacePacketStatistic");

FacePacketStatistic::FacePacketStatistic()
{
  policy = new PacketBasedDroppingPolicy();
  stats  = new SmoothLevelStatistics();
}


FacePacketStatistic::~FacePacketStatistic()
{
  if(policy != NULL)
    delete policy;

  if(stats != NULL)
    delete stats;
}


LevelStatistics* FacePacketStatistic::getStats()
{
  return stats;
}


DroppingPolicy* FacePacketStatistic::getPolicy()
{
  return policy;
}

void FacePacketStatistic::UpdatePolicy()
{
  // Refresh statistics
  this->stats->RefreshStatistics();
  // update (= feed) policy
  this->policy->Feed(*(this->stats));
  // reset stats counters
  this->stats->ResetCounters();
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
