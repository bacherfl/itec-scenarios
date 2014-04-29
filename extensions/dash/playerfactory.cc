#include "playerfactory.h"

using namespace ns3::dashimpl;
using namespace ns3::utils;

NS_LOG_COMPONENT_DEFINE ("PlayerFactory");

PlayerFactory* PlayerFactory::instance = NULL;

PlayerFactory::PlayerFactory()
{
  manager = CreateDashManager();
}

DashPlayer* PlayerFactory::createPlayer(std::string mpd_path, AdaptationLogicType alogic, unsigned int buffer_size,
                                        DownloaderType dwnType, Ptr<Node> node)
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

  utils::Buffer* buffer = new utils::Buffer(buffer_size);

  IAdaptationLogic* logic = resolveAdaptation(alogic, mpd, dataset_path, buffer);
  if(logic == NULL)
  {
    fprintf(stderr, "ERROR: PlayerFactory::createPlayer::logic is NULL\n");
    return NULL;
  }

  DownloadManager *dwnManager = new DownloadManager(dwnType, node);

  return new DashPlayer(mpd, logic, buffer, dwnManager, Names::FindName (node));
}

IAdaptationLogic* PlayerFactory::resolveAdaptation(AdaptationLogicType alogic, dash::mpd::IMPD* mpd, std::string dataset_path, utils::Buffer *buf)
{
  switch(alogic)
  {
    case dashimpl::AlwaysLowest:
      return new AlwaysLowestAdaptationLogic(mpd, dataset_path, buf);
    case dashimpl::RateBased:
      return new RateBasedAdaptationLogic(mpd, dataset_path, buf);
    default:
      return new AlwaysLowestAdaptationLogic(mpd, dataset_path, buf);
  }
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

