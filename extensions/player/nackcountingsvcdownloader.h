#ifndef NACKCOUNTINGSVCDOWNLOADER_H
#define NACKCOUNTINGSVCDOWNLOADER_H

#include "../svc/svcwindowndndownloader.h"

namespace ns3
{
namespace player
{

class NackCountingSVCDownloader : public ns3::utils::SVCWindowNDNDownloader
{
public:
  NackCountingSVCDownloader(std::string& cwnd_type);

  virtual void OnNack (Ptr<const ndn::Interest> interest);
  virtual void OnData (Ptr<const ndn::Data> contentObject);
  virtual bool download(Ptr<utils::Segment> s);

protected:
  int valid_packets;
  int nacks;

  bool checkAbortDownload();
};

}
}

#endif // NACKCOUNTINGSVCDOWNLOADER_H
