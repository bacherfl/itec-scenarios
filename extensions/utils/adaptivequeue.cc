#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/enum.h"
#include "ns3-dev/ns3/uinteger.h"
#include "adaptivequeue.h"
#include "ns3-dev/ns3/ndnSIM/model/wire/ndnsim.h"

#include "../svc/svcleveltag.h"


#include <stdio.h>

NS_LOG_COMPONENT_DEFINE ("AdaptiveQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (AdaptiveQueue);

TypeId AdaptiveQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AdaptiveQueue")
    .SetParent<Queue> ()
    .AddConstructor<AdaptiveQueue> ()
    .AddAttribute ("Mode",
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&AdaptiveQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this AdaptiveQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&AdaptiveQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this AdaptiveQueue.",
                   UintegerValue (100 * 65535),
                   MakeUintegerAccessor (&AdaptiveQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

AdaptiveQueue::AdaptiveQueue () :
  Queue (),
  m_qpackets (),
  m_bytesInQueue (0)
{
  NS_LOG_FUNCTION (this);
}

AdaptiveQueue::~AdaptiveQueue ()
{
  NS_LOG_FUNCTION (this);
}

void
AdaptiveQueue::SetMode (AdaptiveQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

AdaptiveQueue::QueueMode
AdaptiveQueue::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}



bool AdaptiveQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  /*if (m_mode == QUEUE_MODE_PACKETS && (m_qpackets.size () >= m_maxPackets))
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
    {
      NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- droppping pkt");
      Drop (p);
      return false;
    }*/

  ns3::ndn::SVCLevelTag tmpTag, tag;

  if (!p->PeekPacketTag(tag))
    tag.Set(0);



#ifdef LISTQUEUE

  // sorted enqueue

  if (m_qpackets.size() == 0)
  {
    m_qpackets.push_back(p);
  } else {
    int i = 0;

    fprintf(stderr, "AdaptiveQueue Size: %d\n", m_qpackets.size());

    for (std::list < Ptr<Packet> >::iterator ci = m_qpackets.begin();
         ci != m_qpackets.end() && i < m_maxPackets;
         ++ci, i++)
    {
      Ptr < Packet > tmpP = *ci;
      if (!tmpP->PeekPacketTag(tmpTag))
        tmpTag.Set(0);



      // insert before
      if (tag.Get() < tmpTag.Get())
      {
        fprintf(stderr, "AdaptiveQueue Packet accepted - level: %d\n", tag.Get());
        m_qpackets.insert(ci, p);
        break;
      }
    }

    if (i == m_maxPackets)
    {
      // drop that packet
      Drop ( p );
      fprintf(stderr, "AdaptiveQueue dropped ... queue full\n");
      return false;
    } else {
      fprintf(stderr, "AdaptiveQueue Packet with tag %d is on queue position %d (size: %d)\n", tag.Get(), i, m_qpackets.size());
    }

  }


#else
  if (!p->PeekPacketTag(tag))
    tag.Set(0);

  if (tag.Get() != 0 && m_qpackets.size() >= m_maxPackets)
  {
    fprintf(stderr, "dropping packet with tag %d (q size: %d)\n", tag.Get(), m_qpackets.size());

    Drop ( p );

    return false;
  }

  if (m_qpackets.size() >= m_maxPackets*4)
  {
    Drop( p );
    return false;
  }

  m_qpackets.push( p );



  fprintf(stderr, "Packet with tag %d is on queue position ? (size: %d)\n", tag.Get(), m_qpackets.size());
#endif

  m_bytesInQueue += p->GetSize ();


  NS_LOG_LOGIC ("Number packets " << m_qpackets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}



Ptr<Packet>
AdaptiveQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);


  if (m_qpackets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

#ifdef LISTQUEUE
  Ptr<Packet> p = m_qpackets.front ();
  m_qpackets.pop_front();
#else
  Ptr<Packet> p = m_qpackets.top ();
  m_qpackets.pop();
#endif
  m_bytesInQueue -= p->GetSize ();


  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_qpackets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

Ptr<const Packet>
AdaptiveQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_qpackets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
#ifdef LISTQUEUE
  Ptr<Packet> p = m_qpackets.front ();
#else
  Ptr<Packet> p = m_qpackets.top ();
#endif

  NS_LOG_LOGIC ("Number packets " << m_qpackets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

} // namespace ns3
