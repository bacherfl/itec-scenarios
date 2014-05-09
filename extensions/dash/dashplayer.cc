#include "dashplayer.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("DashPlayer");

DashPlayer::DashPlayer(dash::mpd::IMPD* mpd, IAdaptationLogic *alogic, ns3::utils::Buffer* buf, utils::DownloadManager* dwnManager,
                       std::string nodeName)
{
  this->mpd = mpd;
  this->alogic = alogic;
  this->buf = buf;
  this->dwnManager = dwnManager;

  this->dwnManager->addObserver(this);

  isPlaying = false;
  this->m_nodeName = nodeName;
}

void DashPlayer::play ()
{
  NS_LOG_FUNCTION(m_nodeName << this);

  NotifyStart(Simulator::Now().GetSeconds());
  isPlaying = true;
  allSegmentsDownloaded = false;
  this->logDownloadedVideo (mpd->GetMPDPathBaseUrl ()->GetUrl ());
  streaming ();
  Simulator::Schedule(Seconds(2.0), &DashPlayer::consume, this);
}

void DashPlayer::streaming ()
{
  if(isPlaying)
  {
    if(current_segments.size ()== 0)
     current_segments = alogic->getNextSegments();

    //check if last segment
    if(current_segments.size () == 0)
    {
      allSegmentsDownloaded = true;
      return;
    }

   //w8 if buffer is full
    Segment* sample = current_segments.front ();
    if(buf->bufferedSeconds () >= (buf->maxBufferSeconds () - sample->getDuration ()))
    {
      Simulator::Schedule(MilliSeconds (100), &DashPlayer::streaming, this);
      return;
    }

    int requestedLevel = 0;
    //fprintf(stderr, "Requesting SegmentBunch:\n");
    for(int i = 0; i < current_segments.size (); i++)
    {
      //fprintf(stderr, "DashPlayer::requesting Segment: %s\n", current_segments.at(i)->getUri ().c_str ());

      if (current_segments.at(i)->getLevel() > requestedLevel)
      {
        requestedLevel = current_segments.at(i)->getLevel();
      }
    }

    SetRequestedPlayerLevel(current_segments.at(0)->getSegmentNumber(), requestedLevel);

    dlStartTime = Simulator::Now ();
    dwnManager->enque(current_segments);
  }
}

void DashPlayer::stop ()
{
  if(isPlaying)
  {
    NS_LOG_INFO("DashPlayer(" << m_nodeName << "): Stop");
    isPlaying = false;
    NotifyEnd(Simulator::Now().GetSeconds());
    this->WriteToFile(m_nodeName + ".txt");
  }
}

void DashPlayer::update (ObserverMessage msg)
{
  //fprintf(stderr, "NOTIFYED\n");

  switch(msg)
  {
    case Observer::SegmentReceived:
    {
      std::vector<Segment*> received_segs = dwnManager->retriveFinishedSegments ();

      unsigned int total_size = 0;
      Segment* s;
      for(int i = 0; i < received_segs.size (); i++)
      {
        s = received_segs.at (i);
        total_size += s->getSize();
        SetPlayerLevel(s->getSegmentNumber(), s->getLevel(), buf->bufferedSeconds(), s->getSize (), (Simulator::Now ().GetMilliSeconds ()- dlStartTime.GetMilliSeconds ()));
      }

      //fprintf(stderr, "DASH-Player received %d segments for segNumber %u with total size of %u\n", (int)received_segs.size (), received_segs.at(0)->getSegmentNumber(), total_size);

      alogic->updateStatistic (dlStartTime, Simulator::Now (), total_size);

      if(!buf->addData (current_segments.front()->getDuration ()))
      {
        NS_LOG_INFO("DashPlayer(" << m_nodeName << "): BUFFER FULL");
      }

      current_segments.clear ();
      streaming ();
      break;
    }
    default:
    {
      //we are not interested in other events.
      break;
    }

  }


}

void DashPlayer::consume ()
{
  if(allSegmentsDownloaded && buf->isEmpty ())
  {
    NS_LOG_INFO("DashPlayer(" << m_nodeName << "): All Done");
    stop();
    return;
  }

  //fprintf(stderr, "buf->bufferedSeconds = %u \n", buf->bufferedSeconds ());

  if(!buf->consumeData (CONSUME_INTERVALL) && isPlaying)
  {
    logUnsmoothSecond(current_segments.at (0)->getSegmentNumber(),CONSUME_INTERVALL);
    NS_LOG_INFO("DashPlayer(" << m_nodeName << "): CONSUME FAILED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }

  Simulator::Schedule(Seconds (CONSUME_INTERVALL), &DashPlayer::consume, this);
}
