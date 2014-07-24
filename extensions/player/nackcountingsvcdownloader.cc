#include "nackcountingsvcdownloader.h"

using namespace ns3::player;

NS_LOG_COMPONENT_DEFINE ("NackCountingSVCDownloader");

NackCountingSVCDownloader::NackCountingSVCDownloader(std::string &cwnd_type) : SVCWindowNDNDownloader(cwnd_type)
{
}

bool NackCountingSVCDownloader::download(Ptr<utils::Segment> s)
{
  valid_packets = 0;
  nacks = 0;

  return SVCWindowNDNDownloader::download(s);
}

void NackCountingSVCDownloader::OnData (Ptr<const ndn::Data> contentObject)
{
  valid_packets++;
  SVCWindowNDNDownloader::OnData (contentObject);
}

void NackCountingSVCDownloader::OnNack (Ptr<const ndn::Interest> interest)
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
      nacks++;

      if(checkAbortDownload ())
      {
        //NS_LOG_UNCOND("aborted the download of Segment " << segment->getUri () << "\nvalid_packets=" << valid_packets << " nacks=" << nacks);
        abortDownload();
        lastDownloadSuccessful = false;
        notifyAll (utils::Observer::NackReceived);
        return; // stop downloading, do not fire OnNack of super class, we are done here!
      }
    }
  }

  //jumg directly to WindowNDN
  WindowNDNDownloader::OnNack (interest);
}

bool NackCountingSVCDownloader::checkAbortDownload ()
{
  if(valid_packets < 5) // if received less than 5 valid chunks stop download this seg
    return true;

  double ratio = (double) nacks / (double) valid_packets;

  if(ratio > 0.33)
    return true;

  return false;
}
