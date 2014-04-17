#include "playerfactory.h"

using namespace ns3::svc;
using namespace ns3::utils;

PlayerFactory* PlayerFactory::instance = NULL;

PlayerFactory::PlayerFactory()
{
   manager = CreateDashManager();
}

SvcPlayer* PlayerFactory::createPlayer(std::string mpd_path, unsigned int buffer_size, utils::DownloaderType dwnType,
                                       unsigned int maxWidth, unsigned int maxHeight,
                                       Ptr<ns3::Node> node)
{
  dash::mpd::IMPD* mpd = resolveMPD(mpd_path);
  if(mpd == NULL)
  {
    fprintf(stderr, "ERROR: PlayerFactory::createPlayer::mpd is NULL\n");
    return NULL;
  }
  // the dataset_path is needed for simulation purposes only!
  std::string dataset_path = getPWD().append (mpd_path);
  dataset_path = dataset_path.substr (0, dataset_path.find_last_of ('/')+1);

  utils::Buffer* buffer = new utils::Buffer(buffer_size);

  DownloadManager *dwnManager = new DownloadManager(dwnType, node);

  return new SvcPlayer(mpd, dataset_path, dwnManager, buffer, maxWidth, maxHeight, Names::FindName (node));
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


