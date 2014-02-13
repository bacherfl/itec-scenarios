#include "iadaptationlogic.h"

using namespace ns3::dashimpl;

IAdaptationLogic::IAdaptationLogic(dash::mpd::IMPD* mpd)
{
  this->mpd = mpd;
  this->currentPeriod = getFirstPeriod();
  this->currentSegment = NULL;

  this->base_url = this->currentPeriod->GetBaseURLs().at(0)->GetUrl();

  fprintf(stderr, "baseurl %s", base_url.c_str ());
}

dash::mpd::IRepresentation* IAdaptationLogic::getBestRepresentation(dash::mpd::IPeriod* period)
{
  std::vector<dash::mpd::IAdaptationSet *> adaptationSets = period->GetAdaptationSets();

  int bitrate = 0;
  dash::mpd::IRepresentation *best = NULL;

  for(size_t i = 0; i < adaptationSets.size(); i++)
  {
      std::vector<dash::mpd::IRepresentation*> reps = adaptationSets.at(i)->GetRepresentation();
      for(size_t j = 0; j < reps.size(); j++)
      {
          int currentBitrate = reps.at(j)->GetBandwidth();

          if(currentBitrate > bitrate)
          {
              bitrate = currentBitrate;
              best    = reps.at(j);
          }
      }
  }
  return best;
}

dash::mpd::IRepresentation* IAdaptationLogic::getLowestRepresentation(dash::mpd::IPeriod* period)
{
  std::vector<dash::mpd::IAdaptationSet *> adaptationSets = period->GetAdaptationSets();

  int bitrate = 0;
  dash::mpd::IRepresentation *lowest = NULL;

  for(size_t i = 0; i < adaptationSets.size(); i++)
  {
      std::vector<dash::mpd::IRepresentation*> reps = adaptationSets.at(i)->GetRepresentation();
      for(size_t j = 0; j < reps.size(); j++)
      {
          int currentBitrate = reps.at(j)->GetBandwidth();

          if(currentBitrate < bitrate)
          {
              bitrate = currentBitrate;
              lowest    = reps.at(j);
          }
      }
  }
  return lowest;
}

dash::mpd::IPeriod* IAdaptationLogic::getFirstPeriod()
{
  std::vector<dash::mpd::IPeriod *> periods = this->mpd->GetPeriods ();

      if(periods.size() == 0)
          return NULL;

      return periods.at(0);
}

