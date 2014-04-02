#include "simplefilerequester.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SimpleFileRequester);

// register NS-3 type
TypeId SimpleFileRequester::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::SimpleFileRequester")
      .SetParent<ndn::App>()
      .AddConstructor<SimpleFileRequester>()
      .AddAttribute("URI",
                    "URI of the content to request",
                    StringValue("/path/to/sintel-1024-surround.mp4"),
                    MakeStringAccessor(&SimpleFileRequester::URIPath),
                    MakeStringChecker ());
      /*.AddAttribute("InterestPipeline", "The number of maximum pending interests",
                    UintegerValue(8),
                    MakeUintegerAccessor (&DashRequester::interest_pipeline),
                    MakeUintegerChecker<uint32_t> ())*/
      /*.AddAttribute("BufferSize", "The buffer size in seconds.",
                    UintegerValue(20),
                    MakeUintegerAccessor (&DashRequester::buffer_size),
                    MakeUintegerChecker<uint32_t> ());*/
      /*.AddAttribute("SizeOfDataObjects", "The expected size of requested data objects.",
                    UintegerValue(1024),
                    MakeUintegerAccessor (&DashRequester::size_of_data_objects),
                     MakeUintegerChecker<uint32_t> ())*/
  return tid;
}

}

using namespace ns3;



SimpleFileRequester::SimpleFileRequester()
{
  this->downloader = NULL;
}


// Processing upon start of the application
void SimpleFileRequester::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication();

  if (downloader != NULL)
  {
    // stop downloader

    // remove downloader
    delete downloader;
  }

  downloader = new ns3::utils::WindowNDNDownloader();
  downloader->setNodeForNDN(GetNode());

  downloader->download(this->URIPath);

}

// Processing when application is stopped
void SimpleFileRequester::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

// Callback that will be called when Interest arrives
void SimpleFileRequester::OnInterest (Ptr<const ndn::Interest> interest)
{
}

// Callback that will be called when Data arrives
void SimpleFileRequester::OnData (Ptr<const ndn::Data> contentObject)
{
}


