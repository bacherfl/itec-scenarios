#include "svcwindowndndownloader.h"



using namespace ns3;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("SVCWindowNDNDownloader");



SVCWindowNDNDownloader::SVCWindowNDNDownloader() : WindowNDNDownloader()
{
  this->needDownloadBeforeEvent = EventId();
}




void SVCWindowNDNDownloader::OnNack (Ptr<const ndn::Interest> interest)
{
  if(!isPartOfCurrentSegment(interest->GetName ().toUri()))
  {
    fprintf(stderr, "SVCWindow: Dropping NACK from previous Request - URI: %s\n", interest->GetName ().toUri().c_str());
    return;
  }

  //check if packet was dropped on purpose.
  Ptr<Packet> packet = ndn::Wire::FromInterest(interest);
  ndn::SVCLevelTag levelTag;

  bool tagExists = packet->PeekPacketTag(levelTag);
  if (tagExists && levelTag.Get () == -1) //means adaptive node has choosen to drop layers
  {
    NS_LOG_FUNCTION("Packet %s was dropped on purpose\n" << interest->GetName());

    CancelAllTimeoutEvents();
    lastDownloadSuccessful = false;
    notifyAll (Observer::No_Message);
    return; // stop downloading, do not fire OnNack of super class, we are done here!
  }

  // continue with super::OnNack
  WindowNDNDownloader::OnNack(interest);
}





void SVCWindowNDNDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_FUNCTION(this);

  // check if the received URI is what we expected
  if(!isPartOfCurrentSegment(contentObject->GetName().toUri()))
    return;

  // continue with super::OnData
  WindowNDNDownloader::OnData(contentObject);
}




// called, if the download was not finished on time
void SVCWindowNDNDownloader::OnDownloadExpired()
{
  fprintf(stderr, ">>>>>> Download expired - cancelling....\n");


  this->scheduleDownloadTimer.Cancel();

  CancelAllTimeoutEvents();
  lastDownloadSuccessful = false;
  notifyAll (Observer::No_Message);

}


void SVCWindowNDNDownloader::notifyAll(Observer::ObserverMessage msg)
{
  // cancel the download expired event -j ust in case
  this->needDownloadBeforeEvent.Cancel();
  WindowNDNDownloader::notifyAll(msg);
}


bool SVCWindowNDNDownloader::downloadBefore(Segment *s, int miliSeconds)
{
  this->deadline = Simulator::Now().ToInteger(Time::MS) + miliSeconds;

  bool retValue = download(s);

  // this means the last download probably was successful, cancel this event just in case
  this->needDownloadBeforeEvent.Cancel();

  // create a new event
  this->needDownloadBeforeEvent =
      Simulator::Schedule(MilliSeconds(miliSeconds),
                          &SVCWindowNDNDownloader::OnDownloadExpired, this);

  return retValue;
}


bool SVCWindowNDNDownloader::download(Segment *s)
{
  NS_LOG_FUNCTION(this);

  // this means the last download probably was successful, cancel this event just in case
  this->needDownloadBeforeEvent.Cancel();

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

    ndn::DeadlineTag deadlineTag;
    deadlineTag.Set ( this->deadline - Simulator::Now().ToInteger(Time::MS) );
    interest->GetPayload ()->AddPacketTag (deadlineTag);

    // Call trace (for logging purposes)
    m_transmittedInterests (interest, this, m_face);
    m_face->ReceiveInterest (interest);

    packets_sent_this_second++;
  }
}


bool SVCWindowNDNDownloader::isPartOfCurrentSegment(std::string packetUri)
{
  if (!lastDownloadSuccessful)
  {
    return false;
  }

  /*
  fprintf(stderr, "%s \n",curSegmentStatus.base_uri.c_str ());
  fprintf(stderr, "%s \n",packetUri.substr (0,curSegmentStatus.base_uri.size ()).c_str ());
  */

  if (curSegmentStatus.base_uri.compare (packetUri.substr (0,curSegmentStatus.base_uri.size ())) == 0)
  {
    return true;
  }

  return false;
}

DownloaderType SVCWindowNDNDownloader::getDownloaderType ()
{
  return SVCWindowNDN;
}
