#ifndef CONTENTPROVIDER_H
#define CONTENTPROVIDER_H

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/string.h"
#include "ns3-dev/ns3/callback.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/random-variable.h"

#include "ns3-dev/ns3/ndn-app-face.h"
#include "ns3-dev/ns3/ndn-interest.h"
#include "ns3-dev/ns3/ndn-data.h"
#include "ns3-dev/ns3/ndn-fib.h"

#include "utils/idownloader.h"
#include "svc/svcleveltag.h"

#include "ns3-dev/ns3/ndn-wire.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <sstream>
#include <stdio.h>

NS_LOG_COMPONENT_DEFINE ("ContentProvider");

namespace ns3 {

class ContentProvider : public ndn::App
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
  std::string content_path;
  std::string ndn_prefix;
};

} // namespace ns3

#endif // CONTENTPROVIDER_H
