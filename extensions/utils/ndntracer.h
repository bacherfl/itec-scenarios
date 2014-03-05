#ifndef NDNTRACER_H
#define NDNTRACER_H

#include "itracer.h"

#include <stdio.h>

namespace ns3
{
namespace ndn
{
namespace  utils
{

class NDNTracer : public ITracer
{
public:
  NDNTracer(ForwardingStrategy* strategy);

  virtual void OutInterests  (Ptr<const Interest> interest, Ptr<const Face>);

  virtual void InInterests   (Ptr<const Interest> interest, Ptr<const Face> face);

  virtual void DropInterests (Ptr<const Interest> interest, Ptr<const Face> face);

  virtual void OutNacks  (Ptr<const Interest> interest, Ptr<const Face> face);

  virtual void InNacks   (Ptr<const Interest> interest, Ptr<const Face> face);

  virtual void DropNacks (Ptr<const Interest> interest, Ptr<const Face> face);

  virtual void OutData  (Ptr<const Data> data, bool fromCache, Ptr<const Face> face);

  virtual void InData   (Ptr<const Data> data, Ptr<const Face> face);

  virtual void DropData (Ptr<const Data> data, Ptr<const Face> face);

  virtual void SatisfiedInterests (Ptr<const pit::Entry>);

  virtual void TimedOutInterests (Ptr<const pit::Entry>);

};

}
}
}

#endif // NDNTRACER_H
