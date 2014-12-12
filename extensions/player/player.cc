#include "player.h"

using namespace ns3::player;
using namespace ns3::utils;


NS_LOG_COMPONENT_DEFINE ("Player");

Player::Player(dash::mpd::IMPD *mpd,  ns3::dashimpl::IAdaptationLogic *alogic,
               Ptr<LayeredBuffer> buf,
               ns3::utils::DownloadManager* dwnManager, std::string nodeName)
{
  this->mpd = mpd;
  this->alogic = alogic;
  this->buf = buf;
  this->dwnManager = dwnManager;

  this->dwnManager->addObserver(this);

  isPlaying = false;
  this->m_nodeName = nodeName;

  // initialize boolean variables
  isPlaying = false;
  hasStarted = false;
  hasFinished = false;
  currentSegmentNumber = -1;
}


void Player::play ()
{
  NS_LOG_FUNCTION(this << m_nodeName);

  if (hasStarted == true)
  {
    NS_LOG_ERROR("Player(" << m_nodeName << ") tried to start streaming again but already started streaming...");
    return;
  }

  hasStarted = true;
  hasFinished = false;
  isPlaying = false;
  currentSegmentNumber = -1;

  NS_LOG_INFO("Player(" << m_nodeName << ") started streaming");

  NotifyStart(Simulator::Now().GetSeconds());
  logDownloadedVideo (mpd->GetMPDPathBaseUrl ()->GetUrl ());

  // start to stream the video now
  scheduleNextStreaming(0.0);
  // start to at least trying to consume video in CONSUME_DELAY seconds
  scheduleNextConsumeEvent(STARTUP_DELAY);
}

void Player::scheduleNextStreaming (double seconds)
{
  this->streamingEvent = Simulator::Schedule(Seconds(seconds), &Player::streaming, this);
}

void Player::scheduleNextConsumeEvent(double seconds)
{
  this->consumeEvent = Simulator::Schedule(Seconds(seconds), &Player::consuming, this);
}

void Player::stop ()
{
  if (hasStarted == false)
  {
    NS_LOG_ERROR("Player(" << m_nodeName << ") tried to stop streaming but has not started yet...");
    return;
  }

  // reset boolean status variables
  hasStarted = false;
  isPlaying = false;

  // cancel any events
  if (this->streamingEvent.IsRunning ())
  {
    this->streamingEvent.Cancel ();
  }
  if (this->consumeEvent.IsRunning ())
  {
    this->consumeEvent.Cancel ();
  }

  // stop download manager
  dwnManager->stop ();
  NotifyEnd(Simulator::Now().GetSeconds());

  //this writes last stall in log if we stop player before all segments downloaded.
  if (currentSegmentNumber != -1)
  {
    SetConsumedPlayerLevel(this->currentSegmentNumber, 0);
    SetPlayerLevel(currentSegmentNumber, 0, 0, 0,(Simulator::Now ().GetMilliSeconds ()- dlStartTime.GetMilliSeconds ()));
    SetRequestedPlayerLevel(currentSegmentNumber,0);
  }

  WriteToFile(m_nodeName + ".txt");
}


void Player::streaming ()
{
  NS_LOG_FUNCTION(this << m_nodeName);

  // check if we are still supposed to be streaming
  if (hasStarted == true && hasFinished == false)
  {
    // retrieve current_segments (TODO fix thix here, that only 1 segment is returned by adaptation logic)

    std::vector<Ptr<Segment> > segs = alogic->getNextSegments ();
    // check size

    if (segs.size() == 0)
    {
      // nothing to stream at the moment, schedule next streaming event and exit
      NS_LOG_INFO("Player(" << m_nodeName << ") has nothing so stream, waiting...");
      scheduleNextStreaming(0.5);
      return;
    }

    if (segs.size() > 1)
    {
      fprintf(stderr, "ERROR - this player does not support more than one segment at a time\n");
      NS_LOG_ERROR("ERROR - this player does not support more than one segment at a time");

      return;
    }

    current_segment = segs.at (0);

    // set currentSegmentNumber to the first segment if it's not initialized yet, as we always
    // start streaming with the first segment
    if (this->currentSegmentNumber == -1)
    {
      this->currentSegmentNumber = current_segment->getSegmentNumber ();
    }

    // set download start time
    dlStartTime = Simulator::Now ();
    // enque segments
    segs.clear ();
    segs.push_back (current_segment);
    dwnManager->enque(segs);

    // tell our stats collector the same
    SetRequestedPlayerLevel(current_segment->getSegmentNumber(),  current_segment->getLevel ());
  }
}


void Player::consuming ()
{
  NS_LOG_FUNCTION(this << m_nodeName);

  if (hasStarted && !hasFinished && this->currentSegmentNumber != -1)
  {
    // check buffer if we have this->currentSegmentNumber in buffer
    if (this->buf->IsInBuffer (this->currentSegmentNumber, 0))
    {
      if (this->buf->ConsumeFromBuffer (this->currentSegmentNumber, 0))
      {
        //fprintf(stderr, "Consumed segment %d level 0\n", this->currentSegmentNumber);

        int max_level_available = 0;
        // consume all levels
        for (int i = 1; i < buf->GetLevelCount (); i++)
        {
          if (this->buf->ConsumeFromBuffer (this->currentSegmentNumber, i))
          {
            //fprintf(stderr, "Consumed segment %d level %d\n", this->currentSegmentNumber, i);
            max_level_available = i;
          } else {
            break;
          }
        }
        //todo clear unconsumed segments?

        SetConsumedPlayerLevel(this->currentSegmentNumber, max_level_available);

        // tell adaptation logic that we consumed a segment
        this->alogic->segmentConsumed (this->currentSegmentNumber,max_level_available);


        // increase current segment number
        this->currentSegmentNumber++;

        if (this->currentSegmentNumber >= alogic->getNumberOfSegmentsOfCurrenPeriod ())
        {
          // DONE
          hasFinished = true;
          stop();
        } else {
          // schedule next consume in SEGMENT_LENGTH seconds /*Todo* magic number*/
          this->scheduleNextConsumeEvent (2.0);
        }

        // everything is good, return
        return;
      }
    }

    // ELSE:
    // CONSUME FAILED
    //fprintf(stderr, "failed to consume segment nr %d\n", this->currentSegmentNumber);
    this->logUnsmoothSecond (this->currentSegmentNumber, CONSUME_DELAY);

    // else:
    // schedule next try consume in DELAY
    this->scheduleNextConsumeEvent (CONSUME_DELAY);
  }
}




// we received an update message, process it
void Player::update(ObserverMessage msg)
{
  //fprintf(stderr, "Update received!\n");
  switch (msg)
  {
    case No_Message:
    {
      fprintf(stderr, "Update received: No message...\n");
      break;
    }
    case SegmentReceived:
      {
      //fprintf(stderr, "segment received: segmentNr=%d, level=%d\n",
        //      current_segments.at (0)->getSegmentNumber (), current_segments.at (0)->getLevel());
      // received several segments

      // add it to the buffer
      this->buf->AddToBuffer(current_segment->getSegmentNumber (), current_segment->getLevel() );


      // tell adaptation logic that we retrieved this segment
      this->alogic->segmentRetrieved (dlStartTime, Simulator::Now(),
                                      current_segment->getSegmentNumber (),
                                      current_segment->getLevel (),
                                      current_segment->getSize ());

      // tell our stats collector some things
      SetPlayerLevel(current_segment->getSegmentNumber(), current_segment->getLevel(),
                     0, current_segment->getSize (),
                     (Simulator::Now ().GetMilliSeconds ()- dlStartTime.GetMilliSeconds ()));

      // fire streaming
      this->scheduleNextStreaming (0.0);

      break;
      }
    case NackReceived:
    {
      //fprintf(stderr, "Nack received...\n");
      this->alogic->segmentFailed (current_segment->getSegmentNumber (), current_segment->getLevel ());
      this->scheduleNextStreaming (0.0);
      break;
    }
    case SoonFinished:
    {
      fprintf(stderr, "Soon finished...\n");
      break;
    }
    default:
    {
      fprintf(stderr, "Unknown message received...\n");
    }
  }
}

/*overwritten*/
void Player::SetPlayerLevel(unsigned int segmentNumber,
                                        unsigned int level, unsigned int buffer, unsigned int segSize, int64_t dlDuration)
{
 // cout << "SEgmentNumber: " << segmentNumber << ", level=" << level << ", size=" << levelHistory.size() << endl;
  if (levelHistory.find (segmentNumber) == levelHistory.end () ||
     (levelHistory.find (segmentNumber) != levelHistory.end () || levelHistory[segmentNumber] < level) ) //write biggest level
    this->levelHistory[segmentNumber] = level;

  if (bufferHistory.find (segmentNumber) == bufferHistory.end () ||
      (bufferHistory.find (segmentNumber) != bufferHistory.end () && bufferHistory[segmentNumber] > buffer) ) //write smaller buffer
    this->bufferHistory[segmentNumber] = buffer;

  if(segSizeHistory.find (segmentNumber) != segSizeHistory.end ())
    segSizeHistory[segmentNumber] += segSize;
  else
    segSizeHistory[segmentNumber] = segSize;


  if(dlDurationHistory.find (segmentNumber) == dlDurationHistory.end())
    dlDurationHistory[segmentNumber] = dlDuration;
  else
    dlDurationHistory[segmentNumber] += dlDuration;
}
