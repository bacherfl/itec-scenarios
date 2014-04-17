#include "idownloader.h"

using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("IDownloader");

IDownloader::IDownloader()
{
  lastDownloadSuccessful = true;
}


bool IDownloader::wasSuccessfull()
{
  return lastDownloadSuccessful;
}

bool IDownloader::isBussy ()
{
  return bussy;
}

bool IDownloader::downloadFinished ()
{
  return finished;
}

void IDownloader::reset ()
{
  lastDownloadSuccessful = true;
  bussy = false;
  finished = false;
}

