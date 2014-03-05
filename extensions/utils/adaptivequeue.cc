#include "ns3-dev/ns3/log.h"
#include "ns3-dev/ns3/enum.h"
#include "ns3-dev/ns3/uinteger.h"
#include "adaptivequeue.h"
#include "ns3-dev/ns3/ndnSIM/model/wire/ndnsim.h"

#include "svcleveltag.h"


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
  m_packets (),
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



bool AdaptiveQueue::DoNormalEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  if (m_mode == QUEUE_MODE_PACKETS && (m_packets.size () >= m_maxPackets))
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
    }

  m_bytesInQueue += p->GetSize ();
  m_packets.push (p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}


bool AdaptiveQueue::DoSortedEnqueue (Ptr<Packet> p, int level)
{
  NS_LOG_FUNCTION (this << p << level);
  // TODO: Add Adaptation Logic Here
  if (level == 0 || level == 16)
    return DoNormalEnqueue (p);

  NS_LOG_UNCOND ("We dont want that packet with level " << level);
  Drop (p);


  return false;
}


bool
AdaptiveQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);


  std::stringstream os;
  ns3::PacketTagIterator iter = p->GetPacketTagIterator();



  bool normal = true;
  int level = 0;


  while (iter.HasNext())
  {
    ns3::PacketTagIterator::Item item2 = iter.Next();

    if (item2.GetTypeId().GetName().compare("ns3::ndn::SVCLevelTag") == 0)
    {
      ns3::ndn::SVCLevelTag tag;


      item2.GetTag(tag);


      level = tag.Get();
      normal = false;
      break;
    }
    else
    {
      //NS_LOG_UNCOND("TypeName=" << item2.GetTypeId().GetName());

    }
  }

  /*
  PacketMetadata::ItemIterator  it = p->BeginItem();
  while (it.HasNext())
  {
    PacketMetadata::Item item = it.Next ();
    NS_LOG_UNCOND (this << "TYPE=" << item.tid.GetName());


    switch (item.type) {
      case PacketMetadata::Item::HEADER:
      case PacketMetadata::Item::TRAILER:
        if (item.tid.GetName().compare("ns3::ndn::Interest::ndnSIM") == 0)
        {
          NS_ASSERT (item.tid.HasConstructor ());
          Callback<ObjectBase *> constructor = item.tid.GetConstructor ();
          NS_ASSERT (!constructor.IsNull ());
          ObjectBase *instance = constructor ();
          NS_ASSERT (instance != 0);

          ns3::ndn::wire::ndnSIM::Interest *chunk =
              dynamic_cast<ns3::ndn::wire::ndnSIM::Interest *> (instance);

          NS_ASSERT (chunk != 0);
          chunk->Deserialize (item.current);

          Ptr<ndn::Interest> interest = chunk->GetInterest();

          std::string uri = interest->GetName().toUri();

          //NS_LOG_UNCOND (this << "URI=" << uri);


          if (uri.find("-L") != std::string::npos)
          {
            // URI='/itec/bbb/bunny_svc_spatial_2s/bbb-svc.264.seg3-L32.svc/chunk_44'
            uri = uri.substr (uri.find_last_of ("-L")+1);

            uri = uri.substr(0, uri.find_first_of("."));




           // os << "LEVEL='" << uri;
            level = atoi(uri.c_str());
            normal = false;


          }
          else
          {
            // normal packet, just add it
           // os << "Normal Packet" << std::endl;
          }
          //fprintf(stderr, "In Queue %p... %s\n", this, os.str().c_str());

         // chunk->Print(os);

          delete chunk;


        }



        break;
      }

  }

  */


  if (normal == true)
  {
    return DoNormalEnqueue(p);
  } else {
    return DoSortedEnqueue(p, level);
  }
}

Ptr<Packet>
AdaptiveQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();
  m_packets.pop ();
  m_bytesInQueue -= p->GetSize ();

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

Ptr<const Packet>
AdaptiveQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

} // namespace ns3
