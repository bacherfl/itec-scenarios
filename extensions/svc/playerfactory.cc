#include "playerfactory.h"

using namespace ns3::svc;
using namespace ns3::utils;

PlayerFactory* PlayerFactory::instance = NULL;

PlayerFactory::PlayerFactory()
{
}

SvcPlayer* PlayerFactory::createPlayer(std::string mpd_path, unsigned int buffer_size, utils::DownloaderType downloader, Ptr<ns3::Node> node)
{
  return new SvcPlayer();
}

IDownloader* PlayerFactory::resolveDownloader(DownloaderType downloader, Ptr<Node> node)
{
  IDownloader* d = NULL;

  switch(downloader)
  {
    case SimpleNDN:
    {
      d = new SimpleNDNDownloader();
      break;
    }
    case PipelineNDN:
    {
      d = new PipelineNDNDownloader();
      break;
    }
    default:
      d = new SimpleNDNDownloader();
  }

  d->setNodeForNDN (node);
  return d;
}

dash::mpd::IMPD* PlayerFactory::resolveMPD(std::string mpd_path)
{
  return manager->Open( (char*) getPWD().append(mpd_path).c_str());
}


PlayerFactory* PlayerFactory::getInstance ()
{
  if(instance == NULL)
    instance = new PlayerFactory();

  return instance;
}

std::string PlayerFactory::getPWD ()
{
  struct passwd *pw = getpwuid(getuid());
  return std::string(pw->pw_dir);
}


