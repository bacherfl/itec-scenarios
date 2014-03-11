#include "svcbitratetag.h"

namespace ns3 {
namespace ndn {

TypeId
SVCBitrateTag::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::ndn::SVCBitrateTag")
    .SetParent<Tag>()
    .AddConstructor<SVCBitrateTag>()
    ;
  return tid;
}

TypeId
SVCBitrateTag::GetInstanceTypeId () const
{
  return SVCBitrateTag::GetTypeId ();
}

uint32_t
SVCBitrateTag::GetSerializedSize () const
{
  return sizeof(uint32_t);
}

void
SVCBitrateTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_bitrate);
}

void
SVCBitrateTag::Deserialize (TagBuffer i)
{
  m_bitrate = i.ReadU32 ();
}

void
SVCBitrateTag::Print (std::ostream &os) const
{
  os << m_bitrate;
}

} // namespace ndn
} // namespace ns3

