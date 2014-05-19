#include "playerfactory.h"

using namespace ns3;
using namespace ns3::player;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("PlayerFactory");

PlayerFactory* PlayerFactory::instance = NULL;

PlayerFactory::PlayerFactory()
{
  manager = CreateDashManager();
}

Ptr<Player> PlayerFactory::createPlayer(std::string mpd_path, DownloaderType dwnType,  Ptr<Node> node)
{
  dash::mpd::IMPD* mpd = resolveMPD(mpd_path);
  if(mpd == NULL)
  {
    fprintf(stderr, "ERROR: PlayerFactory::createPlayer::mpd %s is NULL\n", mpd_path.c_str ());
    return NULL;
  }
  // the dataset_path is needed for simulation purposes only!
  std::string dataset_path = getPWD().append (mpd_path);
  dataset_path = dataset_path.substr (0, dataset_path.find_last_of ('/')+1);

  DownloadManager *dwnManager = new DownloadManager(dwnType, node);

  return Create<Player>(mpd, dwnManager, Names::FindName (node));
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

