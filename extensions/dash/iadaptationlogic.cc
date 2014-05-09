#include "iadaptationlogic.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("IAdaptationLogic");

IAdaptationLogic::IAdaptationLogic(dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer* buf)
{
  this->mpd = mpd;
  this->dataset_path = dataset_path;
  this->currentPeriod = getFirstPeriod();
  this->currentSegmentNr = 0;
  this->buf = buf;

  if(this->currentPeriod->GetBaseURLs().size () > 0)
    this->base_url = this->currentPeriod->GetBaseURLs().at(0)->GetUrl();
  else
    this->base_url = mpd->GetBaseUrls().at(0)->GetUrl();
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

std::vector<Segment *> IAdaptationLogic::getNextSegments()
{
  dash::mpd::IRepresentation* rep = getOptimalRepresentation(currentPeriod);


  //check if we have svc content or not
  bool stack = false;
  if(rep->GetCodecs ().size () > 0 && rep->GetCodecs().at(0).compare("svc") == 0)
  {
    stack = true;
    //fprintf(stderr, "SVC-Content dedected. Stacking segments..\n");
  }

  std::vector<Segment *> s;
  std::string uri("");
  std::string seg_name("");

  if(rep->GetSegmentList ()->GetSegmentURLs().size() > currentSegmentNr)
  {

    if(stack)
    {
      //we assume here exactly one adaptation set
      std::vector<dash::mpd::IRepresentation*> reps =  currentPeriod->GetAdaptationSets ().at(0)->GetRepresentation();

      for(size_t j = 0; j < reps.size(); j++)
      {
        if(rep->GetId () < reps.at (j)->GetId())
          break;

        uri.clear ();
        seg_name.clear ();

        uri.append (base_url);
        seg_name.append(reps.at(j)->GetSegmentList()->GetSegmentURLs().at(currentSegmentNr)->GetMediaURI());
        uri.append (seg_name);

        s.push_back (new Segment(uri, getFileSize(dataset_path + seg_name),
                                 reps.at(j)->GetSegmentList()->GetDuration(),
                                 reps.at(j)->GetBandwidth (),
                                 atoi(reps.at(j)->GetId ().c_str ()), currentSegmentNr));/*svc level is set to rep level.*/
      }

      currentSegmentNr++;
    }
    else
    {
      uri.append (base_url);
      seg_name.append(rep->GetSegmentList()->GetSegmentURLs().at(currentSegmentNr)->GetMediaURI());
      uri.append (seg_name);

      s.push_back (new Segment(uri, getFileSize(dataset_path + seg_name),
                               rep->GetSegmentList()->GetDuration(),
                               rep->GetBandwidth (), atoi(rep->GetId ().c_str ()), currentSegmentNr));

      currentSegmentNr++;
    }
  }

  return s;
}

dash::mpd::IRepresentation* IAdaptationLogic::getLowestRepresentation(dash::mpd::IPeriod* period)
{
  std::vector<dash::mpd::IAdaptationSet *> adaptationSets = period->GetAdaptationSets();

  int bitrate = INT_MAX;
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

unsigned int IAdaptationLogic::getFileSize (std::string filename)
{
  //remove virtual dataset identifier if exist...
  //filename = filename.substr (0, filename.rfind("-set"));

  struct stat fstats;
  if(!(stat (filename.c_str(), &fstats) == 0))
  {
    fprintf(stderr, "IAdaptationLogic::getFileSize: File does NOT exist: %s\n", filename.c_str ());
    return -1;
  }

  return fstats.st_size;
}

