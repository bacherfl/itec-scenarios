#include "alwayslowestadaptationlogic.h"

using namespace ns3::dashimpl;

AlwaysLowestAdaptationLogic::AlwaysLowestAdaptationLogic(dash::mpd::IMPD *mpd, std::string dataset_path) : IAdaptationLogic(mpd, dataset_path)
{
}

Segment *AlwaysLowestAdaptationLogic::getNextSegmentUri()
{
  dash::mpd::IRepresentation* rep = this->getLowestRepresentation(currentPeriod);

  Segment *s = NULL;
  std::string uri("");
  std::string seg_name("");

  if(rep->GetSegmentList ()->GetSegmentURLs().size() > currentSegmentNr)
  {
    uri.append (base_url);
    seg_name.append(rep->GetSegmentList()->GetSegmentURLs().at(currentSegmentNr)->GetMediaURI());
    uri.append (seg_name);
    currentSegmentNr++;

    s = new Segment(uri, getFileSize(dataset_path + seg_name), rep->GetSegmentList()->GetDuration());
  }

  return s;
}

