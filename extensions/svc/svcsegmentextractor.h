#ifndef SVCSEGMENTEXTRACTOR_H
#define SVCSEGMENTEXTRACTOR_H

#include "libdash/libdash.h"
#include "../utils/segment.h"
#include "../utils/buffer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "ns3/pointer.h"

#include <stdio.h>

#include <vector>

#define CONSIDERD_HISTORY_SIZE 10

#define THREASHOLD 0.2

namespace ns3
{
  namespace svc
  {
    class SVCSegmentExtractor
    {
    public:
      SVCSegmentExtractor(dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer *buf,
                          unsigned int maxWidth, unsigned int maxHeight, uint64_t max_physical_speed);

      std::vector<ns3::Ptr<utils::Segment > > getNextSegments();

      void update(ns3::Ptr<utils::Segment> highest_segment);

    private:
      dash::mpd::IMPD* mpd;
      dash::mpd::IPeriod* currentPeriod;
      unsigned int maxWidth;
      unsigned int maxHeight;
      unsigned int currentSegmentNr;
      std::string dataset_path;
      std::string base_url;

      utils::Buffer* buf;
      uint64_t max_physical_speed;

      std::vector<Ptr<utils::Segment > > highestRequestedHistory;
      std::vector<Ptr<utils::Segment > > highestReceivedHistory;

      std::vector<Ptr<utils::Segment > > considerHistory(std::vector<Ptr<utils::Segment > > segments);
      std::vector<Ptr<utils::Segment > > dropSegments(std::vector<Ptr<utils::Segment > > segments, int max_level);

      dash::mpd::IPeriod* getFirstPeriod();
      unsigned int getFileSize (std::string filename);

    };
}
}

#endif // SVCSEGMENTEXTRACTOR_H
