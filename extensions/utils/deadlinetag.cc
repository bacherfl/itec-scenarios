#include "deadlinetag.h"


namespace ns3 {
namespace ndn {

TypeId
DeadlineTag::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::ndn::DeadlineTag")
    .SetParent<Tag>()
    .AddConstructor<DeadlineTag>()
    ;
  return tid;
}

TypeId
DeadlineTag::GetInstanceTypeId () const
{
  return DeadlineTag::GetTypeId ();
}

uint32_t
DeadlineTag::GetSerializedSize () const
{
  return sizeof(uint64_t);
}

void
DeadlineTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_deadline);
}

void
DeadlineTag::Deserialize (TagBuffer i)
{
  m_deadline = i.ReadU64 ();
}

void
DeadlineTag::Print (std::ostream &os) const
{
  os << m_deadline;
}

} // namespace ndn
} // namespace ns3
