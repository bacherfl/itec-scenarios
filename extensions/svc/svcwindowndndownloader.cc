#include "svcwindowndndownloader.h"

using namespace ns3;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("SVCWindowNDNDownloader");

SVCWindowNDNDownloader::SVCWindowNDNDownloader() : WindowNDNDownloader()
{
  //this->needDownloadBeforeEvent = EventId();
}

void SVCWindowNDNDownloader::OnNack (Ptr<const ndn::Interest> interest)
{
  if(!isPartOfCurrentSegment(interest->GetName ().toUri()))
  {
    //fprintf(stderr, "SVCWindow: Dropping NACK from previous Request - URI: %s\n", interest->GetName ().toUri().c_str());
    return;
  }

  // check if this is level 0 - we are not dropping level 0 = baselayer on purpose.
  if (this->curSegment->getLevel() != 0)
  {
    //check if packet was dropped on purpose.
    Ptr<Packet> packet = ndn::Wire::FromInterest(interest);
    ndn::SVCLevelTag levelTag;

    bool tagExists = packet->PeekPacketTag(levelTag);
    if (tagExists && levelTag.Get () == -1) //means adaptive node has choosen to drop layers
    {
      NS_LOG_UNCOND("Packet %s was dropped on purpose\n" << interest->GetName());

      abortDownload();
      lastDownloadSuccessful = false;
      notifyAll (Observer::NackReceived);
      return; // stop downloading, do not fire OnNack of super class, we are done here!
    }
  }

  // continue with super::OnNack*/
  WindowNDNDownloader::OnNack(interest);
}


// called, if the download was not finished on time
/*void SVCWindowNDNDownloader::OnDownloadExpired()
{
  fprintf(stderr, ">>>>>> Download expired - cancelling....\n");

  this->scheduleDownloadTimer.Cancel();

  CancelAllTimeoutEvents();
  lastDownloadSuccessful = false;
  notifyAll (Observer::No_Message);

}*/


bool SVCWindowNDNDownloader::downloadBefore(Segment *s, int miliSeconds)
{
  /*this->deadline = Simulator::Now().ToInteger(Time::MS) + miliSeconds;

  bool retValue = download(s);

  // this means the last download probably was successful, cancel this event just in case
  this->needDownloadBeforeEvent.Cancel();

  // create a new event
  this->needDownloadBeforeEvent =
      Simulator::Schedule(MilliSeconds(miliSeconds),
                          &SVCWindowNDNDownloader::OnDownloadExpired, this);

  return retValue;*/
  download (s);
}


bool SVCWindowNDNDownloader::download(Segment *s)
{
  NS_LOG_FUNCTION(this);

  this->curSegment = s;

  // this means the last download probably was successful, cancel this event just in case
  //this->needDownloadBeforeEvent.Cancel();

  // set avgBitrate
  curSegmentStatus.avgBitrate = s->getAvgLvlBitrate ();

  return WindowNDNDownloader::download(s);
}


void SVCWindowNDNDownloader::downloadChunk(int chunk_number)
{
  NS_LOG_FUNCTION(this << chunk_number);
  if(this->curSegmentStatus.bytesToDownload != 0)
  {
    Ptr<ndn::Interest> interest = prepareInterestForDownload (chunk_number);

    // extract the string level
    std::string uri = this->curSegmentStatus.base_uri.substr (this->curSegmentStatus.base_uri.find_last_of ("-L")+1);
    uri = uri.substr(0, uri.find_first_of("."));


    int level = atoi(uri.c_str());

    ndn::SVCLevelTag levelTag;
    levelTag.Set(level);
    interest->GetPayload ()->AddPacketTag (levelTag);

    ndn::SVCBitrateTag bitrateTag;
    bitrateTag.Set (curSegmentStatus.avgBitrate);
    interest->GetPayload ()->AddPacketTag (bitrateTag);

    /*ndn::DeadlineTag deadlineTag;
    deadlineTag.Set ( this->deadline - Simulator::Now().ToInteger(Time::MS) );
    interest->GetPayload ()->AddPacketTag (deadlineTag);*/

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest (interest);

    packets_sent_this_second++;
  }
}

DownloaderType SVCWindowNDNDownloader::getDownloaderType ()
{
  return SVCWindowNDN;
}
