#include "downloadmanager.h"

using namespace ns3::utils;

DownloadManager::DownloadManager(DownloaderType dwType, Ptr<Node> node)
{
  //make downloaders ready

  this->node = node;

  IDownloader* d = NULL;
  for(int i = 0; i < DOWNLOADER_NUMBER; i++)
  {
    d = resolveDownloader (dwType, node);

    d->addObserver (this);
    this->downloaders.push_back (d);
  }
}

void DownloadManager::update(ObserverMessage msg)
{
  switch(msg)
  {
    case Observer::NackReceived:
    {
      break;
    }
    case Observer::SegmentReceived://dwnl is still bussy BUT finished and waits for reset
    {
      segmentReceived();
      break;
    }
    case Observer::SoonFinished:
    {
      if(enquedSegments.size () > 0)
        downloadSegments ();
      break;
    }
    default:
    {
      fprintf(stderr, "Received a invalid msg\n");
    }
  }
}

void DownloadManager::addToFinished (Segment *seg)
{
  finishedSegments.push_back (seg);

  if(enquedSegments.size () == 0 && downloaders.size () == getAllNonBussyDownloaders ().size ())
  {
    //bunch of segments finsihed notify observers
    notifyAll (Observer::SegmentReceived);
  }

  return;
}

void DownloadManager::segmentReceived ()
{
  //search for the finished downloader wich where succesfull
   IDownloader *d = NULL;
   for(int i = 0; i < downloaders.size (); i++)
   {
     d = downloaders.at (i);

     if(d->isBussy () && d->downloadFinished () && d->wasSuccessfull ())
     {
       break;
     }
   }

   if(d == NULL)
     return;

   Segment* s = d->getSegment ();

   // reset the downloader state (but not its cwnd)
   d->reset ();

   addToFinished(s);

}

void DownloadManager::enque (std::vector<Segment *> segments)
{
  // ok enque them
  if(segments.size () < 1)
    return;

  this->enquedSegments = segments;
  this->finishedSegments.clear ();

  //if all are non bussy, well trigger the download
  std::vector<IDownloader*> dls = getAllNonBussyDownloaders();

  if(dls.size () == downloaders.size ())
    downloadSegments();
}

void DownloadManager::downloadSegments()
{
  IDownloader* dl = getFreeDownloader ();

  if (dl == NULL)
    return;

  Segment* seg_to_dl = *(enquedSegments.begin ());
  enquedSegments.erase (enquedSegments.begin ());

  dl->download (seg_to_dl);
}

std::vector<IDownloader*> DownloadManager::getAllNonBussyDownloaders()
{
  std::vector<IDownloader*> dwn;

  for(int i = 0; i < downloaders.size (); i++)
    if(!downloaders.at (i)->isBussy())
      dwn.push_back (downloaders.at (i));

  return dwn;
}

std::vector<IDownloader*> DownloadManager::getAllBussyDownloaders()
{
  std::vector<IDownloader*> dwn;

  for(int i = 0; i < downloaders.size (); i++)
    if(downloaders.at (i)->isBussy())
      dwn.push_back (downloaders.at (i));

  return dwn;
}

IDownloader* DownloadManager::getFreeDownloader ()
{
  std::vector<IDownloader*> dls = getAllNonBussyDownloaders();

  if(dls.size () > 0)
    return dls.front ();
  else
    return NULL;
}

std::vector<Segment*> DownloadManager::retriveFinishedSegments()
{
  return finishedSegments;
}

std::vector<Segment *> DownloadManager::retriveUnfinishedSegments()
{
   //we cant return anything just continue download player will stall
  if(finishedSegments.size () < 1)
    return std::vector<Segment*> ();

  // turn off all downloaders segments are too late now
  std::vector<IDownloader*> dwn = getAllBussyDownloaders ();
  for(int i = 0; i < dwn.size (); i++)
  {
    if(!dwn.at(i)->downloadFinished())
    {
      dwn.at(i)->abortDownload();
      dwn.at(i)->reset();
    }
  }

  std::vector<Segment*> return_segs = finishedSegments;
  enquedSegments.clear ();
  finishedSegments.clear ();

  return return_segs;
}

IDownloader* DownloadManager::resolveDownloader(DownloaderType downloader, Ptr<Node> node)
{
  IDownloader* d = NULL;

  switch(downloader)
  {
    case SimpleNDN:
    {
      d = new SimpleNDNDownloader();
      break;
    }
    case WindowNDN:
    {
      d = new WindowNDNDownloader();
      break;
    }
    default:
      d = new WindowNDNDownloader();
  }

  d->setNodeForNDN (node);
  return d;
}

uint64_t DownloadManager::getPhysicalBitrate()
{
  // Get Device Bitrate
  Ptr<PointToPointNetDevice> nd1 =
  node->GetDevice(0)->GetObject<PointToPointNetDevice>();
  DataRateValue dv;
  nd1->GetAttribute("DataRate", dv);
  DataRate d = dv.Get();
  return d.GetBitRate();
}

