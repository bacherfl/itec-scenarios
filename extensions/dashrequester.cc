#include "dashrequester.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DashRequester);

// register NS-3 type
TypeId DashRequester::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::DashRequester")
      .SetParent<ndn::App>()
      .AddConstructor<DashRequester>()
      .AddAttribute("MPD",
                    "Path to MPD file.",
                    StringValue("/path/to/mpd"),
                    MakeStringAccessor(&DashRequester::mpd_path),
                    MakeStringChecker ())
      .AddAttribute("InterestPipeline", "The number of maximum pending interests",
                    UintegerValue(8),
                    MakeUintegerAccessor (&DashRequester::interest_pipeline),
                    MakeUintegerChecker<uint32_t> ())
      .AddAttribute("BufferSize", "The buffer size in seconds.",
                    UintegerValue(20),
                    MakeUintegerAccessor (&DashRequester::buffer_size),
                    MakeUintegerChecker<uint32_t> ())
      .AddAttribute("SizeOfDataObjects", "The expected size of requested data objects.",
                    UintegerValue(1024),
                    MakeUintegerAccessor (&DashRequester::size_of_data_objects),
                     MakeUintegerChecker<uint32_t> ());
  return tid;
}

}

using namespace ns3;

// Processing upon start of the application
void DashRequester::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();

  manager = CreateDashManager();
  mpd = manager->Open ((char*) getPWD().append(mpd_path).c_str());

  if(mpd == NULL)
  {
    NS_LOG_WARN("DashRequester::StartApplication: Opening of MPD FAILED, " << this);
    this->StopApplication ();
  }

  //parse the pseudeo mpd
  //create a buffer
  //create an adaptation logic

}

// Processing when application is stopped
void DashRequester::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();

  if(manager != NULL)
    manager->Delete ();

  if(buf != NULL)
    delete buf;
}

// Callback that will be called when Interest arrives
void DashRequester::OnInterest (Ptr<const ndn::Interest> interest)
{
}

// Callback that will be called when Data arrives
void DashRequester::OnData (Ptr<const ndn::Data> contentObject)
{
}

std::string DashRequester::getPWD ()
{
  struct passwd *pw = getpwuid(getuid());
  return std::string(pw->pw_dir);
}
