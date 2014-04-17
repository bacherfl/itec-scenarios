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
  isPlaying = true;
  allSegmentsDownloaded = false;
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

    fprintf(stderr, "Requesting SegmentBunch:\n");
    for(int i = 0; i < current_segments.size (); i++)
    {
      fprintf(stderr, "DashPlayer::requesting Segment: %s\n", current_segments.at(i)->getUri ().c_str ());
    }

    dlStartTime = Simulator::Now ();
    dwnManager->enque(current_segments);
  }
}

void DashPlayer::stop ()
{
  NS_LOG_INFO("DashPlayer(" << m_nodeName << "): Stop");
  this->WriteToFile(m_nodeName + ".txt");
  isPlaying = false;
}

void DashPlayer::update (ObserverMessage msg)
{
  //fprintf(stderr, "NOTIFYED\n");

  std::vector<Segment*> received_segs = dwnManager->retriveFinishedSegments ();

  unsigned int total_size = 0;

  for(int i = 0; i < received_segs.size (); i++)
  {
    total_size += received_segs.at (i)->getSize();
    SetPlayerLevel(received_segs.at(i)->getSegmentNumber(), received_segs.at(i)->getLevel(), buf->bufferedSeconds());
  }

  fprintf(stderr, "DASH-Player received %d segments for segNumber %u with total size of %u\n", (int)received_segs.size (), received_segs.at(0)->getSegmentNumber(), total_size);

  alogic->updateStatistic (dlStartTime, Simulator::Now (), total_size);

  if(!buf->addData (current_segments.front()->getDuration ()))
  {
    NS_LOG_INFO("DashPlayer(" << m_nodeName << "): BUFFER FULL");
  }

  current_segments.clear ();
  streaming ();
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
    NS_LOG_UNCOND("DashPlayer(" << m_nodeName << "): CONSUME FAILED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    //fprintf(stderr, "CONSUMED FAILED\n");
  }

  Simulator::Schedule(Seconds (CONSUME_INTERVALL), &DashPlayer::consume, this);
}
