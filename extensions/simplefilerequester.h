#ifndef SIMPLEFILEREQUESTER_H
#define SIMPLEFILEREQUESTER_H

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/string.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"

#include "dash/playerfactory.h"
#include "dash/dashplayer.h"
#include "dash/iadaptationlogic.h"
#include "utils/idownloader.h"
#include "utils/segment.h"

NS_LOG_COMPONENT_DEFINE ("SimpleFileRequester");
namespace ns3 {

class SimpleFileRequester : public ndn::App
{
public:
  SimpleFileRequester();
  // register NS-3 type "SimpleFileRequester"
  static TypeId GetTypeId ();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void StartApplication ();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void StopApplication ();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void OnInterest (Ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void OnData (Ptr<const ndn::Data> contentObject);


protected:
  std::string URIPath;
  utils::WindowNDNDownloader* downloader;


};
}

#endif // SIMPLEFILEREQUESTER_H
