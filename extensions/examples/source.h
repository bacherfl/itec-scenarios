#ifndef SORUCE_H_
#define SORUCE_H_

#include "ns3-dev/ns3/ndn-app.h"
#include <cstdio>

namespace ns3 {

class Source : public ndn::App
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
  void SendInterest ();
};

} // namespace ns3

#endif // CUSTOM_APP_H_
