#ifndef SVCBITRATETAG_H
#define SVCBITRATETAG_H

#include "ns3-dev/ns3/tag.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-fw
 * @brief Packet tag that is used to track hop count for Interest-Data pairs
 */
class SVCBitrateTag : public Tag
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * @brief Default constructor
   */
  SVCBitrateTag ()  { m_bitrate = 0; }

  /**
   * @brief Destructor
   */
  ~SVCBitrateTag () { }


  /**
   * @brief Set value of level
   */
  void
  Set (int32_t bitrate) { m_bitrate = bitrate; }

  /**
   * @brief Get value of level
   */
  int32_t
  Get () const { return m_bitrate; }

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
  int32_t m_bitrate;
};

} // namespace ndn
} // namespace ns3

#endif // SVCBITRATETAG_H
