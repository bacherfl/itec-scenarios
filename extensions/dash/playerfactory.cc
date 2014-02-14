#include "playerfactory.h"

using namespace ns3::dashimpl;

PlayerFactory* PlayerFactory::instance = NULL;

PlayerFactory::PlayerFactory()
{
  manager = CreateDashManager();
}

DashPlayer* PlayerFactory::createPlayer(std::string mpd_path, AdaptationLogicType alogic, unsigned int buffer_size)
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

  IAdaptationLogic* logic = resolveAdaptation(alogic, mpd, dataset_path);

  if(logic == NULL)
  {
    fprintf(stderr, "ERROR: PlayerFactory::createPlayer::logic is NULL\n");
    return NULL;
  }

  utils::Buffer* buffer = new utils::Buffer(buffer_size);

  return new DashPlayer(mpd, logic, buffer);
}

IAdaptationLogic* PlayerFactory::resolveAdaptation(AdaptationLogicType alogic, dash::mpd::IMPD* mpd, std::string dataset_path)
{
  switch(alogic)
  {
    case dashimpl::AlwaysLowest:
      return new AlwaysLowestAdaptationLogic(mpd, dataset_path);
    default:
      return new AlwaysLowestAdaptationLogic(mpd, dataset_path);
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

