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

unsigned int FacePITStatistic::getResidualBandwidth ()
{
  Ptr<PointToPointNetDevice> d = face->GetNode()->GetDevice(0)->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  d->GetAttribute("DataRate", dv);
  DataRate rate = dv.Get();

  uint64_t availableBitrate = rate.GetBitRate();

  uint64_t residualBandwidth = 0;

  if(availableBitrate > AVERAGE_DATAPACKET_SIZE * pendingInterestCount)
    residualBandwidth = AVERAGE_DATAPACKET_SIZE * pendingInterestCount;

  return residualBandwidth;
}
