#ifndef SVCSEGMENTEXTRACTOR_H
#define SVCSEGMENTEXTRACTOR_H

#include "libdash/libdash.h"
#include "../utils/segment.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <stdio.h>

#include <vector>

namespace ns3
{
  namespace svc
  {
    class SVCSegmentExtractor
    {
    public:
      SVCSegmentExtractor(dash::mpd::IMPD* mpd, std::string dataset_path, unsigned int maxWidth, unsigned int maxHeight);

      std::vector<utils::Segment*> getNextSegments();

    private:
      dash::mpd::IMPD* mpd;
      dash::mpd::IPeriod* currentPeriod;
      unsigned int maxWidth;
      unsigned int maxHeight;
      unsigned int currentSegmentNr;
      std::string dataset_path;
      std::string base_url;

      dash::mpd::IPeriod* getFirstPeriod();
      unsigned int getFileSize (std::string filename);

    };
}
}

#endif // SVCSEGMENTEXTRACTOR_H
