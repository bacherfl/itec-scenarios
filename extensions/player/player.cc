#include "player.h"

using namespace ns3::player;
using namespace ns3::utils;


NS_LOG_COMPONENT_DEFINE ("Player");

Player::Player(dash::mpd::IMPD *mpd,  ns3::dashimpl::LayeredAdaptationLogic  *alogic,
               ns3::player::LayeredBuffer* buf,
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
  scheduleNextConsumeEvent(CONSUME_DELAY);
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
  WriteToFile(m_nodeName + ".txt");
}


void Player::streaming ()
{
  NS_LOG_FUNCTION(this << m_nodeName);

  // check if we are still supposed to be streaming
  if (hasStarted == true && hasFinished == false)
  {
    // retrieve current_segments (TODO)
    this->current_segments = this->alogic->getNextSegments ();

    // check size
    if (current_segments.size() == 0)
    {
      // nothing to stream at the moment, schedule next streaming event and exit
      NS_LOG_INFO("Player(" << m_nodeName << ") has nothing so stream, waiting...");
      scheduleNextStreaming(0.5);
      return;
    }

    if (current_segments.size() > 1)
    {
      fprintf(stderr, "ERROR - this player does not support more than one segment at a time\n");
      NS_LOG_ERROR("ERROR - this player does not support more than one segment at a time");

      return;
    }



#define DEBUG

#ifdef DEBUG
    // get list:
    std::stringstream listSegments;
#endif

    for(int i = 0; i < current_segments.size (); i++)
    {
#ifdef DEBUG
      listSegments << listSegments << "(S:" << current_segments.at (i)->getSegmentNumber () << ", L:" <<
          current_segments.at (i)->getLevel () << "), ";
#endif
    }

#ifdef DEBUG
    NS_LOG_INFO("Player(" << m_nodeName << ") streaming segments " << listSegments.str().c_str ());
#endif

    // set currentSegmentNumber to the first segment if it's not initialized yet, as we always
    // start streaming with the first segment
    if (this->currentSegmentNumber == -1)
    {
      this->currentSegmentNumber = current_segments.at (0)->getSegmentNumber ();
    }

    // set download start time
    dlStartTime = Simulator::Now ();
    // enque segments
    dwnManager->enque(current_segments);

    // tell our stats collector the same
    SetRequestedPlayerLevel(current_segments.at(0)->getSegmentNumber(),  current_segments.at (0)->getLevel ());
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
        for (int i = 1; i < 6; i++)
        {
          if (this->buf->ConsumeFromBuffer (this->currentSegmentNumber, i))
          {
            //fprintf(stderr, "Consumed segment %d level %d\n", this->currentSegmentNumber, i);
            max_level_available = i;
          } else {
            break;
          }
        }

        SetConsumedPlayerLevel(this->currentSegmentNumber, max_level_available);

        // tell adaptation logic that we consumed a segment
        this->alogic->segmentConsumed (this->currentSegmentNumber);


        // increase current segment number
        this->currentSegmentNumber++;

        if (this->currentSegmentNumber >= 299)
        {
          // DONE
          hasFinished = true;
          stop();
        } else {
          // schedule next consume in SEGMENT_LENGTH seconds
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

  /* No_Message,
          SegmentReceived,
          NackReceived,
          SoonFinished */

  switch (msg)
  {
    case No_Message:
      fprintf(stderr, "Update received: No message...\n");
      break;
    case SegmentReceived:
      //fprintf(stderr, "segment received: segmentNr=%d, level=%d\n",
        //      current_segments.at (0)->getSegmentNumber (), current_segments.at (0)->getLevel());
      // received several segments

      // add it to the buffer
      this->buf->AddToBuffer(current_segments.at (0)->getSegmentNumber (), current_segments.at (0)->getLevel() );

      // tell adaptation logic that we retrieved this segment
      this->alogic->segmentRetrieved (dlStartTime, Simulator::Now(),
                                      current_segments.at (0)->getSegmentNumber (),
                                      current_segments.at (0)->getLevel (),
                                      current_segments.at (0)->getSize ());
      // tell our stats collector some things
      SetPlayerLevel(current_segments.at (0)->getSegmentNumber(), current_segments.at (0)->getLevel(),
                     0, current_segments.at (0)->getSize (),
                     (Simulator::Now ().GetMilliSeconds ()- dlStartTime.GetMilliSeconds ()));


      // remove from current_segments
      current_segments.clear();

      // fire streaming
      this->scheduleNextStreaming (0.0);

      break;
    case NackReceived:
      fprintf(stderr, "Nack received...\n");
      break;
    case SoonFinished:
      fprintf(stderr, "Soon finished...\n");
      break;
    default:
      fprintf(stderr, "Unknown message received...\n");

  }


}
