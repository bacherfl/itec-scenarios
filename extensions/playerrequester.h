#ifndef PLAYERREQUESTER_H
#define PLAYERREQUESTER_H

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/string.h"
#include "ns3-dev/ns3/integer.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"
#include "player/player.h"
#include "player/playerfactory.h"

#include "utils/idownloader.h"

NS_LOG_COMPONENT_DEFINE ("PlayerRequester");

namespace ns3 {

  class PlayerRequester  : public ndn::App
  {
  public:
    // register NS-3 type "CustomApp"
    static TypeId GetTypeId ();

    // (overridden from ndn::App) Processing upon start of the application
    virtual void StartApplication ();

    // (overridden from ndn::App) Processing when application is stopped
    virtual void StopApplication ();

    // (overridden from ndn::App) Callback that will be called when Interest arrives
    virtual void OnInterest (Ptr<const ndn::Interest> interest);

    // (overridden from ndn::App) Callback that will be called when Data arrives
    virtual void OnData (Ptr<const ndn::Data> contentObject);

  private:
    std::string mpd_path;

    int enableAdaptation;

    Ptr<ns3::player::Player> player;

    std::string cwnd_type;
  };
}
#endif // PLAYERREQUESTER_H
