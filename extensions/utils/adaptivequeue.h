#ifndef ADAPTIVEQUEUE_H
#define ADAPTIVEQUEUE_H


#include <queue>
#include "ns3-dev/ns3/packet.h"
#include "ns3-dev/ns3/queue.h"
#include "ns3-dev/ns3/core-module.h"
#include "ns3-dev/ns3/network-module.h"
#include "ns3-dev/ns3/ndnSIM-module.h"
#include "ns3-dev/ns3/point-to-point-module.h"


namespace ns3 {

class TraceContainer;

/**
 * \ingroup queue
 *
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class AdaptiveQueue : public Queue {
public:
  static TypeId GetTypeId (void);
  /**
   * \brief AdaptiveQueue Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  AdaptiveQueue ();

  virtual ~AdaptiveQueue();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (AdaptiveQueue::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  AdaptiveQueue::QueueMode GetMode (void);

private:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual bool DoNormalEnqueue (Ptr<Packet> p);
  virtual bool DoSortedEnqueue (Ptr<Packet> p, int level);

  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  std::queue<Ptr<Packet> > m_packets;
  uint32_t m_maxPackets;
  uint32_t m_maxBytes;
  uint32_t m_bytesInQueue;
  QueueMode m_mode;
};

} // namespace ns3

#endif // ADAPTIVEQUEUE_H
