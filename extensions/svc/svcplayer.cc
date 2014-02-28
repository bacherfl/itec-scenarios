#include "svcplayer.h"

using namespace ns3::svc;

SvcPlayer::SvcPlayer(dash::mpd::IMPD *mpd, std::string dataset_path, utils::IDownloader *downloader,
                     utils::Buffer *buf, unsigned int maxWidth, unsigned int maxHeight)
{
  this->mpd = mpd;
  this->downloader = downloader;
  this->buf = buf;
  this->extractor = new SVCSegmentExtractor(mpd, dataset_path, maxWidth, maxHeight);
}

void SvcPlayer::play()
{
  std::vector<utils::Segment*> segments = extractor->getNextSegments ();

  for(int i = 0; i < segments.size (); i++)
  {
    utils::Segment* s = segments.at (i);
    fprintf(stderr, "SegmentUri %s SegmentLevel %d\n", s->getUri ().c_str (), s->getLevel ());
  }

}


void SvcPlayer::stop()
{
}

