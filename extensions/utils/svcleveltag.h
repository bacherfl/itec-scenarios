#ifndef SVCLEVELTAG_H
#define SVCLEVELTAG_H


#include "ns3-dev/ns3/tag.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-fw
 * @brief Packet tag that is used to track hop count for Interest-Data pairs
 */
class SVCLevelTag : public Tag
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * @brief Default constructor
   */
  SVCLevelTag ()  { m_level = 0; }

  /**
   * @brief Destructor
   */
  ~SVCLevelTag () { }


  /**
   * @brief Set value of level
   */
  void
  Set (int32_t level) { m_level = level; }

  /**
   * @brief Get value of level
   */
  int32_t
  Get () const { return m_level; }

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
  int32_t m_level;
};

} // namespace ndn
} // namespace ns3



#endif // SVCLEVELTAG_H
