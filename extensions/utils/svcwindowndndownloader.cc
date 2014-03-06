#include "svcwindowndndownloader.h"



using namespace ns3;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("SVCWindowNDNDownloader");



SVCWindowNDNDownloader::SVCWindowNDNDownloader() : WindowNDNDownloader()
{
}




void SVCWindowNDNDownloader::OnNack (Ptr<const ndn::Interest> interest)
{
  if(!isPartOfCurrentSegment(interest->GetName ().toUri()))
  {
    fprintf(stderr, "SVCWindow: Dropping wrong NACK - URI: %s\n", interest->GetName ().toUri().c_str());
    return;
  }

  fprintf(stderr, "SVCWindow: NACK - URI: %s\n", interest->GetName ().toUri().c_str());

  //check if packet was dropped on purpose.
  Ptr<Packet> packet = ndn::Wire::FromInterest(interest);
  ndn::SVCLevelTag levelTag;

  bool tagExists = packet->PeekPacketTag(levelTag);
  if (tagExists && levelTag.Get () == -1) //means adaptive node has choosen to drop layers
  {
    NS_LOG_FUNCTION("NACK %s was dropped on purpose\n" << interest->GetName());

    CancelAllTimeoutEvents();

    lastDownloadSuccessful = false;

    notifyAll ();
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




bool SVCWindowNDNDownloader::isPartOfCurrentSegment(std::string packetUri)
{
  if (!lastDownloadSuccessful)
  {
    //fprintf(stderr, "RETURN false1\n");
    return false;
  }

  /*
  fprintf(stderr, "%s \n",curSegmentStatus.base_uri.c_str ());
  fprintf(stderr, "%s \n",packetUri.substr (0,curSegmentStatus.base_uri.size ()).c_str ());
  */

  if (curSegmentStatus.base_uri.compare (packetUri.substr (0,curSegmentStatus.base_uri.size ())) == 0)
  {
    //fprintf(stderr, "RETURN true2\n");
    return true;
  }

  //fprintf(stderr, "RETURN false3\n");
  return false;
}
