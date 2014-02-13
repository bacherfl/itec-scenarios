#ifndef BACKGROUNDTRAFFIC_H
#define BACKGROUNDTRAFFIC_H

#include "ns3-dev/ns3/ndn-app.h"
#include "ns3-dev/ns3/simulator.h"
#include "ns3-dev/ns3/string.h"
#include "ns3-dev/ns3/callback.h"
#include "ns3-dev/ns3/ptr.h"
#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/random-variable.h"

#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"

#include "ns3-dev/ns3/ndn-app-face.h"
#include "ns3-dev/ns3/ndn-app-helper.h"
#include "ns3-dev/ns3/ndn-data.h"

#include "ns3-dev/ns3/ndnSIM/apps/ndn-consumer.h"
#include "ns3-dev/ns3/ndnSIM/apps/ndn-producer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>


namespace ns3 {

class BackgroundTraffic
{
public:
  BackgroundTraffic();
  BackgroundTraffic(float start, float stop);
  BackgroundTraffic(float start, float stop, std::string prefix);
  ~BackgroundTraffic();

  // register NS-3 type "BackgroundTraffic"
  static TypeId GetTypeId ();
  void Install(const NodeContainer& producerList, const NodeContainer& consumerList, ndn::GlobalRoutingHelper& ndnGlobalRoutingHelper);


protected:
  double      m_bgtrafficstart;
  double      m_bgtrafficstop;
  ndn::AppHelper*  m_consumerHelper;
  ndn::AppHelper*  m_producerHelper;

private:
  std::string content_provider;
  std::string ndn_prefix;
};

} // namespace ns3

#endif // BACKGROUNDTRAFFIC_H
