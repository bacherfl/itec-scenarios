#include "playerrequester.h"

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (PlayerRequester);

// register NS-3 type
TypeId PlayerRequester::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::PlayerRequester")
      .SetParent<ndn::App>()
      .AddConstructor<PlayerRequester>()
      .AddAttribute("MPD",
                    "Path to MPD file.",
                    StringValue("/path/to/mpd"),
                    MakeStringAccessor(&PlayerRequester::mpd_path),
                    MakeStringChecker ());
  return tid;
}

// Processing upon start of the application
void PlayerRequester::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();
  player = player::PlayerFactory::getInstance()->createPlayer(mpd_path, utils::WindowNDN, this->GetNode ());
  player->play();
}

// Processing when application is stopped
void PlayerRequester::StopApplication ()
{
  if (player)
  {
    player->stop ();
  }
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

// Callback that will be called when Interest arrives
void PlayerRequester::OnInterest (Ptr<const ndn::Interest> interest)
{
}

// Callback that will be called when Data arrives
void PlayerRequester::OnData (Ptr<const ndn::Data> contentObject)
{
}

