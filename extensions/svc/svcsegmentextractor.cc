#include "svcsegmentextractor.h"

using namespace ns3::svc;
using namespace ns3::utils;

SVCSegmentExtractor::SVCSegmentExtractor(dash::mpd::IMPD* mpd, std::string dataset_path, Buffer *buf,
                                         unsigned int maxWidth, unsigned int maxHeight, uint64_t max_physical_speed)
{
  this->mpd = mpd;
  this->maxWidth = maxWidth;
  this->maxHeight = maxHeight;
  this->dataset_path = dataset_path;
  this->max_physical_speed = max_physical_speed;
  this->buf = buf;

  this->currentPeriod = getFirstPeriod();

  currentSegmentNr = 0;

  if(this->currentPeriod->GetBaseURLs().size () > 0)
    this->base_url = this->currentPeriod->GetBaseURLs().at(0)->GetUrl();
  else
    this->base_url = mpd->GetBaseUrls().at(0)->GetUrl();
}

std::vector<Segment*> SVCSegmentExtractor::getNextSegments()
{
  std::vector<dash::mpd::IAdaptationSet*> sets = currentPeriod->GetAdaptationSets ();
  dash::mpd::IAdaptationSet* set = sets.at (0); //Todo deal with different sets

  std::vector<dash::mpd::IRepresentation*> reps = set->GetRepresentation ();

  int width, height;
  dash::mpd::IRepresentation* rep;

  std::vector<Segment*> segments;

  for(size_t j = 0; j < reps.size(); j++)
  {
    rep = reps.at(j);

    width = rep->GetWidth();
    height = rep->GetHeight();

    std::string uri("");
    std::string seg_name("");

    if(width <= maxWidth && height <= maxHeight /*resolution check*/
       && rep->GetBandwidth () <= max_physical_speed /*physical_bandwidth_check*/)
    {
      if(rep->GetSegmentList ()->GetSegmentURLs().size() > currentSegmentNr)
        {
          uri.append (base_url);
          seg_name.append(rep->GetSegmentList()->GetSegmentURLs().at(currentSegmentNr)->GetMediaURI());
          uri.append (seg_name);

          segments.push_back (
                new Segment(uri, getFileSize(dataset_path + seg_name),
                            rep->GetSegmentList()->GetDuration(), rep->GetBandwidth (),
                            atoi(rep->GetId ().c_str ()), currentSegmentNr));
        }
    }
  }

  if(segments.size () == 0) //means we are finished no more segments to fetch.
    return segments;

  segments = considerHistory (segments);

  highestRequestedHistory.push_back (segments.back ());
  currentSegmentNr++;

  return segments;
}

std::vector<Segment*> SVCSegmentExtractor::considerHistory(std::vector<Segment*> segments)
{
  int history_size = highestReceivedHistory.size ();

  //if history is small request just the lowest level
  if(history_size == 0)
  {
    //fprintf(stderr, "History to small for consideration\n");
    return dropSegments(segments, 0);
  }

  if(history_size > CONSIDERD_HISTORY_SIZE )
    history_size = CONSIDERD_HISTORY_SIZE;

  //Do something usefull here
  double avg_requested_level = 0.0;
  double avg_received_level = 0.0;

  for(int i = highestReceivedHistory.size () - 1; i > highestReceivedHistory.size () - history_size - 1; i--)
  {
    avg_requested_level += highestRequestedHistory.at (i)->getLevel();
    avg_received_level += highestReceivedHistory.at (i)->getLevel();
  }

  avg_received_level /= history_size ;
  avg_requested_level /= history_size ;

   double max_level = (avg_requested_level - (avg_requested_level - avg_received_level));

   fprintf(stderr, "avg_requested_level = %f\n", avg_requested_level);
   fprintf(stderr, "avg_received_level = %f\n", avg_received_level);
   fprintf(stderr, "buf->fillPercentage () = %f\n", buf->fillPercentage ());
   fprintf(stderr, "max_level = %f\n", max_level);

  if(max_level + THREASHOLD >= avg_requested_level && buf->fillPercentage () > 0.5)
    max_level++;
  else if(max_level > 0 && buf->fillPercentage () < 0.25)
    max_level--;

  fprintf(stderr, "Dropping segments with level > %d\n", (int) max_level);

  segments = dropSegments (segments, max_level);



  return segments;
}

std::vector<Segment*> SVCSegmentExtractor::dropSegments(std::vector<Segment*> segments, int max_level)
{
  for(std::vector<Segment*>::iterator it = segments.begin (); it != segments.end (); )
  {
    if((*it)->getLevel() > max_level)
      segments.erase (it);
    else
      it++;
  }
  return segments;
}

void SVCSegmentExtractor::update(utils::Segment *highest_segment)
{
  if(highestRequestedHistory.back ()->getSegmentNumber () != highest_segment->getSegmentNumber ())
  {
    NS_LOG_UNCOND("SVCSegmentExtractor::update INVALID update for " << highestRequestedHistory.back ()->getSegmentNumber () <<
                  " requested, but Segment " << highest_segment->getSegmentNumber () << " received");
    return;
  }

  highestReceivedHistory.push_back (highest_segment);
}

// check which segments are feasible to be download and remove the rest

dash::mpd::IPeriod* SVCSegmentExtractor::getFirstPeriod()
{
  std::vector<dash::mpd::IPeriod *> periods = this->mpd->GetPeriods ();

      if(periods.size() == 0)
          return NULL;

      return periods.at(0);
}

unsigned int SVCSegmentExtractor::getFileSize (std::string filename)
{
  struct stat fstats;
  if(!(stat (filename.c_str(), &fstats) == 0))
  {
    fprintf(stderr, "SVCSegmentExtractor::OnInterest: File does NOT exist: %s\n", filename.c_str ());
    return -1;
  }

  return fstats.st_size;
}
