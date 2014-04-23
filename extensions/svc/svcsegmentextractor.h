#ifndef SVCSEGMENTEXTRACTOR_H
#define SVCSEGMENTEXTRACTOR_H

#include "libdash/libdash.h"
#include "../utils/segment.h"
#include "../utils/buffer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <stdio.h>

#include <vector>

#define CONSIDERD_HISTORY_SIZE 10

namespace ns3
{
  namespace svc
  {
    class SVCSegmentExtractor
    {
    public:
      SVCSegmentExtractor(dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer *buf,
                          unsigned int maxWidth, unsigned int maxHeight, uint64_t max_physical_speed);

      std::vector<utils::Segment*> getNextSegments();

      void update(utils::Segment* highest_segment);

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

      std::vector<utils::Segment*> highestRequestedHistory;
      std::vector<utils::Segment*> highestReceivedHistory;

      std::vector<utils::Segment*> considerHistory(std::vector<utils::Segment*> segments);

      dash::mpd::IPeriod* getFirstPeriod();
      unsigned int getFileSize (std::string filename);

    };
}
}

#endif // SVCSEGMENTEXTRACTOR_H
