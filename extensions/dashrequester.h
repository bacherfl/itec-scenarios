#ifndef DASHREQUESTER_H
#define DASHREQUESTER_H

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/string.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"

#include "dash/playerfactory.h"
#include "dash/dashplayer.h"
#include "dash/iadaptationlogic.h"
#include "utils/idownloader.h"

NS_LOG_COMPONENT_DEFINE ("DashRequester");
namespace ns3 {

class DashRequester : public ndn::App
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
  unsigned int buffer_size;
  std::string mpd_path;

  ns3::dashimpl::DashPlayer *player;

};

}
#endif // DASHREQUESTER_H
