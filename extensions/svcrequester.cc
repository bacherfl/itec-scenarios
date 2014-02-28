#include "svcrequester.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SvcRequester);

// register NS-3 type
TypeId SvcRequester::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::SvcRequester")
      .SetParent<ndn::App>()
      .AddConstructor<SvcRequester>()
      .AddAttribute("MPD",
                    "Path to MPD file.",
                    StringValue("/path/to/mpd"),
                    MakeStringAccessor(&SvcRequester::mpd_path),
                    MakeStringChecker ())
      .AddAttribute("BufferSize", "The buffer size in seconds.",
                    UintegerValue(20),
                    MakeUintegerAccessor (&SvcRequester::buffer_size),
                    MakeUintegerChecker<uint32_t> ());
  return tid;
}

}

using namespace ns3;

// Processing upon start of the application
void SvcRequester::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();
  player = svc::PlayerFactory::getInstance()->createPlayer(mpd_path, 30, utils::PipelineNDN, 1280, 720, GetNode());
  player->play();
}

// Processing when application is stopped
void SvcRequester::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();

  if (player)
    delete player;
}

// Callback that will be called when Interest arrives
void SvcRequester::OnInterest (Ptr<const ndn::Interest> interest)
{
}

// Callback that will be called when Data arrives
void SvcRequester::OnData (Ptr<const ndn::Data> contentObject)
{
}
