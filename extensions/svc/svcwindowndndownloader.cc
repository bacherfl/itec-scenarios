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
  if (this->segment->getLevel() != 0)
  {
    //check if packet was dropped on purpose.
    Ptr<Packet> packet = ndn::Wire::FromInterest(interest);
    ndn::SVCLevelTag levelTag;

    bool tagExists = packet->PeekPacketTag(levelTag);
    if (tagExists && levelTag.Get () == -1) //means adaptive node has choosen to drop layers
    {
      NS_LOG_INFO("Packet was dropped on purpose:" << interest->GetName());

      if (this->curSegmentStatus.bytesToDownload > 0)
      {
          //fprintf(stderr, "Dropping segment with level %d, chunks=%d, bytesToDownload=%d\n", this->curSegment->getLevel(), this->curSegmentStatus.num_chunks, this->curSegmentStatus.bytesToDownload);
      }

      abortDownload();
      lastDownloadSuccessful = false;
      notifyAll (Observer::NackReceived);
      return; // stop downloading, do not fire OnNack of super class, we are done here!
    }
  }
  // instead, we are forwarding that nack to the super class
  // the super class will then most likely reduce the congestion window to avoid more congestion

  // continue with super::OnNack*/
  // WindowNDNDownloader::OnNack(interest);
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


bool SVCWindowNDNDownloader::downloadBefore(Ptr<Segment> s, int miliSeconds)
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


bool SVCWindowNDNDownloader::download(Ptr<Segment> s)
{
  NS_LOG_FUNCTION(this);

  this->segment = s;

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
    // check that we actually need this chunk, it might have been received by now
    while (this->curSegmentStatus.chunk_status[chunk_number] == Received)
    {
      chunk_number = GetNextNeededChunkNumber ();
      if (chunk_number == -1)
      {
        // we seem to be done for now
        break;
      }
    }

    // make sure we have a valid chunk_number to request
    if (chunk_number != -1)
    {
      prepareInterestForDownload (chunk_number);

      // extract the string level
      std::string uri = this->curSegmentStatus.base_uri.substr (this->curSegmentStatus.base_uri.find_last_of ("-L")+1);
      uri = uri.substr(0, uri.find_first_of("."));


      int level = atoi(uri.c_str());

      ndn::SVCLevelTag levelTag;
      levelTag.Set(level);
      interest->GetPayload ()->AddPacketTag (levelTag);

      /*
      ndn::SVCBitrateTag bitrateTag;
      bitrateTag.Set (curSegmentStatus.avgBitrate);
      interest->GetPayload ()->AddPacketTag (bitrateTag);
        */
      /*ndn::DeadlineTag deadlineTag;
      deadlineTag.Set ( this->deadline - Simulator::Now().ToInteger(Time::MS) );
      interest->GetPayload ()->AddPacketTag (deadlineTag);*/

      // Call trace (for logging purposes)
      m_transmittedInterests (interest, this, m_face);
      m_face->ReceiveInterest (interest);

      packets_sent_this_second++;
    }
  }

  // else: either bytesToDownload = 0 or chunk_number was -1, in both cases:
  chunk_number = this->GetNextNeededChunkNumber ();

  if(chunk_number == -1 && !soonFinishedAlreadyFired)
  {
    soonFinishedAlreadyFired = false;
    notifyAll(Observer::SoonFinished);
  }
}

DownloaderType SVCWindowNDNDownloader::getDownloaderType ()
{
  return SVCWindowNDN;
}

