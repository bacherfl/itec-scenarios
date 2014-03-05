#include "svcleveltag.h"

namespace ns3 {
namespace ndn {

TypeId
SVCLevelTag::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::ndn::SVCLevelTag")
    .SetParent<Tag>()
    .AddConstructor<SVCLevelTag>()
    ;
  return tid;
}

TypeId
SVCLevelTag::GetInstanceTypeId () const
{
  return SVCLevelTag::GetTypeId ();
}

uint32_t
SVCLevelTag::GetSerializedSize () const
{
  return sizeof(uint32_t);
}

void
SVCLevelTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_level);
}

void
SVCLevelTag::Deserialize (TagBuffer i)
{
  m_level = i.ReadU32 ();
}

void
SVCLevelTag::Print (std::ostream &os) const
{
  os << m_level;
}

} // namespace ndn
} // namespace ns3
