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
  IAdaptationLogic* logic = resolveAdaptation(alogic, mpd);
  utils::Buffer* buffer = new utils::Buffer(buffer_size);


  return new DashPlayer(mpd, logic, buffer);
}

IAdaptationLogic* PlayerFactory::resolveAdaptation(AdaptationLogicType alogic, dash::mpd::IMPD* mpd)
{
  switch(alogic)
  {
    //todo
    default:
    return new AlwaysLowestAdaptationLogic(mpd);
  }
}

dash::mpd::IMPD* PlayerFactory::resolveMPD(std::string mpd_path)
{
  return manager->Open((char*)mpd_path.c_str ());
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

