#include "simplebufferlogic.h"

using namespace ns3::player;

SimpleBufferLogic::SimpleBufferLogic(dash::mpd::IMPD *mpd, std::string dataset_path, Ptr<player::LayeredBuffer> buf)
  : IAdaptationLogic(mpd, dataset_path, NULL)
{
  this->buf = buf;
  this->last_consumed_segment_number = -1;
  this->max_level = this->getRepresentationsOrderdById ().size() - 1;
}

dash::mpd::IRepresentation* SimpleBufferLogic::getOptimalRepresentation (dash::mpd::IPeriod *period)
{
  std::vector<dash::mpd::IRepresentation*> reps = this->getRepresentationsOrderdById();

  for(unsigned int level = 0; level <= max_level; level++) // itererate over levels
  {
    //check if level reached the required buffer
    if(buf->BufferSize (level) < DESIRED_BUFFER_SIZE_IN_SEGMENTS)
    {
      return reps.at (level);
    }
  }
  //fprintf(stderr, "IDLE....\n");
  return NULL; // all level-buffers are full
}

void SimpleBufferLogic::segmentRetrieved(Time start, Time stop,
                              unsigned int segment_number, unsigned int segment_level, unsigned int segment_size)
{

}

void SimpleBufferLogic::segmentFailed(unsigned int segment_number, unsigned int segment_level)
{

}

void SimpleBufferLogic::segmentConsumed(unsigned int segment_number, unsigned int segment_level)
{
  last_consumed_segment_number = segment_number;
}

unsigned int SimpleBufferLogic::getNextNeededSegmentNumber(int level)
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
      // request last consumed + offset
      return this->last_consumed_segment_number + OFFSET;
    }
  } else {
    // get last segment number for level and add +1
    return this->buf->LastSegmentNumber(level) + 1;
  }
}

std::vector<ns3::Ptr<ns3::utils::Segment> > SimpleBufferLogic::getNextSegments()
{
  dash::mpd::IRepresentation* rep = getOptimalRepresentation(currentPeriod);

  std::vector<Ptr<utils::Segment> > s;
  std::string uri("");
  std::string seg_name("");

  if (rep != NULL)
  {
    int level = atoi(rep->GetId ().c_str ());
    int nextSegNumber = getNextNeededSegmentNumber(level);

    if(rep->GetSegmentList ()->GetSegmentURLs().size() > nextSegNumber)
    {
      uri.append (base_url);
      seg_name.append(rep->GetSegmentList()->GetSegmentURLs().at(nextSegNumber)->GetMediaURI());
      uri.append (seg_name);

      s.push_back (Create<utils::Segment>(uri, getFileSize(dataset_path + seg_name),
                               rep->GetSegmentList()->GetDuration(),
                               rep->GetBandwidth (), atoi(rep->GetId ().c_str ()), nextSegNumber));

    }
  }

  return s;
}

