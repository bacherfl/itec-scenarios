#include "backgroundtraffic.h"

NS_LOG_COMPONENT_DEFINE ("ns3.BackgroundTraffic");

namespace ns3{


  BackgroundTraffic::BackgroundTraffic()
  {
    BackgroundTraffic(1.0, 2.0, "/");
  }

  BackgroundTraffic::BackgroundTraffic(float start, float stop)
  {
    BackgroundTraffic(start, stop, "/");
  }

  BackgroundTraffic::BackgroundTraffic(float start, float stop, std::string prefix)
  {
    this->m_bgtrafficstart = start;
    this->m_bgtrafficstop  = stop;
    this->ndn_prefix       = prefix;

    // create default consumer helper
    this->m_consumerHelper = new ndn::AppHelper("ns3::ndn::ConsumerCbr");
    m_consumerHelper->SetPrefix (this->ndn_prefix);
    m_consumerHelper->SetAttribute ("Frequency", StringValue("30")); // 30 interests a second (ca. 1MBit traffic)
    m_consumerHelper->SetAttribute("Randomize", StringValue("exponential")); // distribute requests exponentially

    // create default producer helper
    this->m_producerHelper = new ndn::AppHelper("ns3::ndn::Producer");
    m_producerHelper->SetPrefix (this->ndn_prefix);
    m_producerHelper->SetAttribute ("PayloadSize", StringValue("4096"));
  }


  BackgroundTraffic::~BackgroundTraffic()
  {
    NS_LOG_UNCOND("Destroying background traffic");
    delete this->m_consumerHelper;
    delete this->m_producerHelper;
  }




  void BackgroundTraffic::Install(const NodeContainer& producerList, const NodeContainer& consumerList, ndn::GlobalRoutingHelper& ndnGlobalRoutingHelper)
  {
      NS_LOG_FUNCTION(this);
      NS_LOG_UNCOND("In InstallAll() of backgroundtraffic");
      NS_LOG_INFO("BGTRafficStart:" << this->m_bgtrafficstart << ", " << this->m_bgtrafficstop << ";");




      ApplicationContainer dummyConsumer = m_consumerHelper->Install(consumerList);
      dummyConsumer.Start (Seconds(this->m_bgtrafficstart));
      dummyConsumer.Stop (Seconds(this->m_bgtrafficstop));



      ApplicationContainer dummyProducer = m_producerHelper->Install(producerList);
      dummyProducer.Start (Seconds(this->m_bgtrafficstart-0.5));
      dummyProducer.Stop (Seconds(this->m_bgtrafficstop+5)); // make sure to have the producer available for a little bit longer
      ndnGlobalRoutingHelper.AddOrigins(this->ndn_prefix, producerList);




  }


}
