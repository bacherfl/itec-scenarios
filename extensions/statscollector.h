#ifndef STATSCOLLECTORS_H
#define STATSCOLLECTORS_H

#include <ns3/ndn-forwarding-strategy.h>
#include <ns3/log.h>

#include <stdio.h>

namespace ns3 {
namespace ndn {
namespace fw {

template<class Parent>
class StatsCollector :
    public Parent
{
  typedef Parent super;

public:

  static TypeId GetTypeId ();
    
  static std::string GetLogName ();

  StatsCollector (){}

  protected:

  virtual void DidSendOutInterest (Ptr<Face> inFace, Ptr<Face> outFace,
                      Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry);

private:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // STATSCOLLECTORS_H
