#include "svcsegmentextractor.h"

using namespace ns3::svc;
using namespace ns3::utils;

SVCSegmentExtractor::SVCSegmentExtractor(dash::mpd::IMPD* mpd, std::string dataset_path, unsigned int maxWidth, unsigned int maxHeight)
{
  this->mpd = mpd;
  this->maxWidth = maxWidth;
  this->maxHeight = maxHeight;
  this->dataset_path = dataset_path;

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

    if(width <= maxWidth && height <= maxHeight)
    {
      if(rep->GetSegmentList ()->GetSegmentURLs().size() > currentSegmentNr)
        {
          uri.append (base_url);
          seg_name.append(rep->GetSegmentList()->GetSegmentURLs().at(currentSegmentNr)->GetMediaURI());
          uri.append (seg_name);

          segments.push_back (new Segment(uri, getFileSize(dataset_path + seg_name), rep->GetSegmentList()->GetDuration(), rep->GetBandwidth (), atoi(rep->GetId ().c_str ())));
        }
    }
  }
  currentSegmentNr++;

  return segments;
}

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
