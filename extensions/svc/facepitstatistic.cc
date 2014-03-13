#include "facepitstatistic.h"

using namespace ns3::ndn::svc;

NS_LOG_COMPONENT_DEFINE ("FacePITStatistic");

FacePITStatistic::FacePITStatistic(Ptr<Face> face)
{
  pendingInterestCount = 0;
  this->face = face;
}

void FacePITStatistic::increase ()
{
  pendingInterestCount++;
}

void FacePITStatistic::decrease ()
{
  if(pendingInterestCount > 0)
    pendingInterestCount--;
  else
    NS_LOG_ERROR("PendingInterestCount is already 0. Can't decrease it.");
}

unsigned int FacePITStatistic::getPendingInterestCount()
{
  return pendingInterestCount;
}
