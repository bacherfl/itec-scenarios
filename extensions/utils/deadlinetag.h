#ifndef DEADLINETAG_H
#define DEADLINETAG_H


#include "ns3-dev/ns3/tag.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-fw
 * @brief Packet tag that is used to track hop count for Interest-Data pairs
 */
class DeadlineTag : public Tag
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * @brief Default constructor
   */
  DeadlineTag ()  { m_deadline = 0; }

  /**
   * @brief Destructor
   */
  ~DeadlineTag () { }


  /**
   * @brief Set value of level
   */
  void
  Set (uint64_t deadline) { m_deadline = deadline; }

  /**
   * @brief Get value of level
   */
  int64_t
  Get () const { return m_deadline; }

  ////////////////////////////////////////////////////////
  // from ObjectBase
  ////////////////////////////////////////////////////////
  virtual TypeId
  GetInstanceTypeId () const;

  ////////////////////////////////////////////////////////
  // from Tag
  ////////////////////////////////////////////////////////

  virtual uint32_t
  GetSerializedSize () const;

  virtual void
  Serialize (TagBuffer i) const;

  virtual void
  Deserialize (TagBuffer i);

  virtual void
  Print (std::ostream &os) const;

private:
  uint64_t m_deadline;
};

} // namespace ndn
} // namespace ns3


#endif // DEADLINETAG_H
