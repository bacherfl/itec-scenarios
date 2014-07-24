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

Ptr<Player> PlayerFactory::createPlayer(std::string mpd_path, DownloaderType dwnType, std::string& cwnd_type, dashimpl::AdaptationLogicType atype, Ptr<Node> node)
{
  dash::mpd::IMPD* mpd = resolveMPD(mpd_path);
  if(mpd == NULL)
  {
    NS_LOG_ERROR("PlayerFactory::createPlayer: resolveMPD(" << mpd_path << ") returned NULL");
    return NULL;
  }
  // the dataset_path is needed for simulation purposes only!
  std::string dataset_path = getPWD().append (mpd_path);
  dataset_path = dataset_path.substr (0, dataset_path.find_last_of ('/')+1);

  DownloadManager *dwnManager = new DownloadManager(dwnType, cwnd_type, node);
  Ptr<LayeredBuffer> buffer = Create<LayeredBuffer>();
  dashimpl::IAdaptationLogic* alogic = resolveAdaptation (atype, mpd, dataset_path,buffer);

  return Create<Player>(mpd, alogic, buffer, dwnManager, Names::FindName (node));
}

dashimpl::IAdaptationLogic* PlayerFactory::resolveAdaptation(dashimpl::AdaptationLogicType alogic, dash::mpd::IMPD* mpd, std::string dataset_path, Ptr<LayeredBuffer> buffer)
{
  switch(alogic)
  {
    case dashimpl::LayerdSieber:
      return new ns3::dashimpl::LayeredAdaptationLogic(mpd, dataset_path, buffer);
    case dashimpl::SimpleBuffer:
      return new ns3::player::SimpleBufferLogic(mpd, dataset_path, buffer);
    default:
      return new ns3::dashimpl::LayeredAdaptationLogic(mpd, dataset_path, buffer);
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

