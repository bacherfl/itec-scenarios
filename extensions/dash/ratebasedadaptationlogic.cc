#include "ratebasedadaptationlogic.h"

using namespace ns3::dashimpl;

NS_LOG_COMPONENT_DEFINE ("RateBasedAdaptationLogic");

RateBasedAdaptationLogic::RateBasedAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, utils::Buffer *buf) : IAdaptationLogic(mpd, dataset_path, buf)
{
  avg_dl_rates = (int64_t*) calloc(SEGMENT_HISTORY, sizeof (int64_t));
  index = 0;
}

dash::mpd::IRepresentation* RateBasedAdaptationLogic::getOptimalRepresentation(dash::mpd::IPeriod *period)
{
  std::vector<dash::mpd::IAdaptationSet*> sets = period->GetAdaptationSets ();
  dash::mpd::IAdaptationSet* set = sets.at (0); //Todo deal with different sets

  std::vector<dash::mpd::IRepresentation*> reps = set->GetRepresentation ();

  dash::mpd::IRepresentation* rep = getLowestRepresentation (period);
  unsigned int avg = getAvgDLS();

  for(int i = 0; i < reps.size (); i++)
  {
    if(reps.at(i)->GetBandwidth() <= avg && reps.at(i)->GetBandwidth() > rep->GetBandwidth ())
      rep = reps.at (i);
  }

  return rep;
}

unsigned int RateBasedAdaptationLogic::getAvgDLS ()
{
  int64_t avg = 0;
  int counter = 0;
  for(int i = 0; i < SEGMENT_HISTORY; i++)
    if(avg_dl_rates[i] != 0)
    {
      avg += avg_dl_rates[i];
      counter++;
    }

  avg /= counter;

  if(counter > 0)
    return avg * buf->fillPercentage ();
  else
    return 0;
}

void RateBasedAdaptationLogic::updateStatistic(Time start, Time stop, unsigned int segment_size)
{
  //fprintf(stderr, "update\n");
  if(index >= SEGMENT_HISTORY)
    index = 0;
  int64_t dif = (stop-start).GetMilliSeconds ();
  //fprintf(stderr, "dif ms = %d\n", dif);
  int64_t bits = segment_size * 8; //total bits
  //fprintf(stderr, "segmentsize bits = %d\n", bits);
  bits = bits * 1000;
  bits = bits / dif; // 1000 * bits / ms = bits / s
  //fprintf(stderr, "avg bits = %d\n", bits);
  //bits = bits * 0.80; //introduce 20% safety

  //fprintf(stderr, "avg kbits = %d\n", bits);

  avg_dl_rates[index++] = bits;
}

