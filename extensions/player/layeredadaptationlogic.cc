#include "layeredadaptationlogic.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;


#include <stdio.h>


#define BUFFER_MIN_SIZE 3
#define BUFFER_ALPHA 3


#define MAX_LEVEL 5


LayeredAdaptationLogic::LayeredAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path, Ptr<player::LayeredBuffer> buf)
 : IAdaptationLogic(mpd, dataset_path, NULL)
{
  this->buf = buf;

  this->last_consumed_segment_number = -1;


  averageBandwidth.resize(MAX_LEVEL+1, 0);

  // calculate avg bandwidth (independent per layer)
  for (int i = 0; i <= MAX_LEVEL; i++)
  {
    int bandwidth = getAvgBandwidthForLayer(i);
    averageBandwidth.at(i) = bandwidth;
  }

  // make it independent
  for (int i = 1; i <= MAX_LEVEL; i++)
  {
    int sum = 0;
    for (int j = i+1; j <= MAX_LEVEL; j++)
    {
      sum += averageBandwidth.at(j);
    }
    averageBandwidth.at(i) = averageBandwidth.at(i) - sum;
  }
}

/*
dash::mpd::IRepresentation* LayeredAdaptationLogic::getOptimalRepresentation (dash::mpd::IPeriod *period)
{
  return NULL;
}
*/


unsigned int LayeredAdaptationLogic::desired_buffer_size(int i, int i_curr)
{
  return BUFFER_MIN_SIZE + (i_curr - i) * BUFFER_ALPHA;
}


unsigned int LayeredAdaptationLogic::getNextNeededSegmentNumber(int level)
{
  unsigned int next_segment_number = -1;

  // check buffer
  if (this->buf->BufferSize (level) == 0)
  {
    // empty buffer, check if level = 0
    if (level == 0)
    {
      // level 0 and buffer empty? Means we need to get the last consuemd one +1
      return this->last_consumed_segment_number + 1;
    } else {
      // level != 0 and buffer empty? means we "should" be in quality increase phase
      // request last consumed + BUFFER_MIN_SIZE
      return this->last_consumed_segment_number + BUFFER_MIN_SIZE;
    }
  } else {
    // buffer not empty
    // get last segment number for level and add +1
    return this->buf->LastSegmentNumber(level) + 1;
  }
}


dash::mpd::IRepresentation* LayeredAdaptationLogic::getOptimalRepresentation (dash::mpd::IPeriod *period)
{
  /*
  std::vector<dash::mpd::IAdaptationSet*> sets = period->GetAdaptationSets ();
  dash::mpd::IAdaptationSet* set = sets.at (0); //Todo deal with different sets

  std::vector<dash::mpd::IRepresentation*> reps = set->GetRepresentation ();
  */


  std::vector<dash::mpd::IRepresentation*> reps = this->getRepresentationsOrderdById();

  int i = 0;

  // Get i_curr
  int i_curr = reps.size()-1;

  unsigned int next_segment_number = -1;


  while (this->buf->BufferSize (i_curr) == 0 && i_curr > 0)
  {
    i_curr--;
  }

  // Steady Phase
  while (i <= i_curr)
  {
    if (this->buf->BufferSize (i) < desired_buffer_size(i, i_curr))
    {
      // i is the next, need segment number though
      next_segment_number = getNextNeededSegmentNumber(i);

      //fprintf(stderr, "Steady Phase: The next segment: SegNr=%d, Level=%d\n", next_segment_number, i);

      this->currentSegmentNr = next_segment_number;
      return reps.at(i);
    }

    i++;
  }

  i = 0;

  // Growing Phase
  while (i <= i_curr)
  {
    if (this->buf->BufferSize (i) < desired_buffer_size(i, i_curr+2))
    {
      // i is the next, need segment number though
      next_segment_number = getNextNeededSegmentNumber(i);

      //fprintf(stderr, "Growing Phase: The next segment: SegNr=%d, Level=%d\n", next_segment_number, i);

      this->currentSegmentNr = next_segment_number;
      return reps.at(i);
    }

    i++;
  }

  // Quality Increase Phase
  if (i != reps.size()-1)
  {
    i_curr++;
    next_segment_number = getNextNeededSegmentNumber(i);

    //fprintf(stderr, "Quality Increase: The next segment: SegNr=%d, Level=%d\n", next_segment_number, i);

    this->currentSegmentNr = next_segment_number;
    return reps.at(i);
  } else {
    //fprintf(stderr, "Quality Increase: IDLE....\n");
    return NULL;
  }

  return getLowestRepresentation(period);
}


/*   std::vector<dash::mpd::IAdaptationSet*> sets = period->GetAdaptationSets ();
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

  */



void LayeredAdaptationLogic::segmentRetrieved(Time start, Time stop,
                              unsigned int segment_number, unsigned int segment_level, unsigned int segment_size)
{

}

void LayeredAdaptationLogic::segmentFailed(unsigned int segment_number, unsigned int segment_level)
{

}


void LayeredAdaptationLogic::segmentConsumed(unsigned int segment_number)
{
  this->last_consumed_segment_number = segment_number;
}


std::vector<ns3::Ptr<Segment> > LayeredAdaptationLogic::getNextSegments()
{
  dash::mpd::IRepresentation* rep = getOptimalRepresentation(currentPeriod);


  std::vector<Ptr<Segment> > s;
  std::string uri("");
  std::string seg_name("");

  if (rep != NULL)
  {
    if(rep->GetSegmentList ()->GetSegmentURLs().size() > currentSegmentNr)
    {
      uri.append (base_url);
      seg_name.append(rep->GetSegmentList()->GetSegmentURLs().at(currentSegmentNr)->GetMediaURI());
      uri.append (seg_name);

      s.push_back (Create<Segment>(uri, getFileSize(dataset_path + seg_name),
                               rep->GetSegmentList()->GetDuration(),
                               rep->GetBandwidth (), atoi(rep->GetId ().c_str ()), currentSegmentNr));

    }
  }

  return s;
}

