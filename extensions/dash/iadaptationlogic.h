#ifndef IADAPTATIONLOGIC_H
#define IADAPTATIONLOGIC_H

#include <string>
#include <vector>
#include "libdash/libdash.h"

#include "ns3-dev/ns3/timer.h"
#include "ns3/pointer.h"

#include <boost/lexical_cast.hpp>

#include "../utils/buffer.h"
#include "../utils/segment.h"

#include <limits.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <vector>
#include <algorithm>

namespace ns3
{
  namespace dashimpl
  {

    /*!
     * \brief Enumeration that specifies the type AdaptationLogic type.
     */
    enum AdaptationLogicType
    {
        AlwaysBest, //todo impl
        AlwaysLowest,
        RateBased
    };

    /*!
     * \brief The class IAdaptationLogic serves as interface and as abstract class for all AdaptationLogics.
     */
    class IAdaptationLogic
    {

    public:

      /*!
       * \brief IAdaptationLogic creates new AdaptationLogic
       * \param mpd The MPD.
       * \param dataset_path The path to the DASH dataset.
       * \param buf
       */
      IAdaptationLogic(dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer *buf);

      /*!
       * \brief Deconstructor
       */
      virtual ~IAdaptationLogic(){}

      /*!
       * \brief Gets the next n Segments which should be downloaded. If we have single layer encoded data n = 1. If we have multi layer content like svc n >= 1. Leading segments have represent low quality layers.
       * \return A vector of Segments.
       */
      virtual std::vector<Ptr<utils::Segment> > getNextSegments();

      /*!
       * \brief Updates the statistics on which adaptation desicions are made.
       * \param start Start time of the download of a segment.
       * \param stop End time of the download of a segment.
       * \param segment_number the number of the segment retrieved
       * \param segment_level the level of the segment retrieved
       * \param segment_size The size of the downloaded segment.
       */
      virtual void segmentRetrieved(Time start, Time stop,
                                    unsigned int segment_number, unsigned int segment_level, unsigned int segment_size) = 0;

      /*!
        * \brief notifys the logic that we failed to receive a certain segment
        * \param segment_number the segment number of the segment we failed to receive
        * \param segment_level the level of the segment we failed to receive
      */
      virtual void segmentFailed(unsigned int segment_number, unsigned int segment_level) = 0;

      virtual int getAvgBandwidthForLayer(unsigned int segment_level);


    protected:
      dash::mpd::IMPD* mpd;
      std::string dataset_path;
      dash::mpd::IPeriod* currentPeriod;
      unsigned int currentSegmentNr;
      std::string base_url;
      utils::Buffer* buf;

      /*!
       * \brief This function should be overwirten by Childclasses. It returns the optimal Representation according to the AdaptationLogic.
       * \param Current Period in the MPD of streaming.
       * \return Optimal Representation.
       */
      virtual dash::mpd::IRepresentation* getOptimalRepresentation(dash::mpd::IPeriod *period) = 0;

      /*!
       * \brief Extracts the first Period of a MPD.
       * \return
       */
      virtual dash::mpd::IPeriod* getFirstPeriod();

      /*!
       * \brief Extracts the best (highest) Representation of the given Period.
       * \param period A period of the MPD.
       * \return
       */
      virtual dash::mpd::IRepresentation* getBestRepresentation(dash::mpd::IPeriod* period);

      /*!
       * \brief Extracts the lowest Representation of the given Period.
       * \param period A period of the MPD.
       * \return
       */
      virtual dash::mpd::IRepresentation* getLowestRepresentation(dash::mpd::IPeriod* period);

      /*!
       * \brief Gets the size of a File.
       * \param filename Path to the file.
       * \return
       */
      virtual unsigned int getFileSize(std::string filename);

    };
  }
}
#endif // IADAPTATIONLOGIC_H
