#include "playerrequester.h"

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (PlayerRequester);

// register NS-3 type
TypeId PlayerRequester::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ndn::PlayerRequester")
      .SetParent<ndn::App> ()
      .AddConstructor<PlayerRequester> ()
      .AddAttribute("MPD",
                    "Path to MPD file.",
                    StringValue ("/path/to/mpd"),
                    MakeStringAccessor (&PlayerRequester::mpd_path),
                    MakeStringChecker ())
      .AddAttribute("EnableAdaptation",
                    "0 or 1",
                    IntegerValue (0),
                    MakeIntegerAccessor (&PlayerRequester::enableAdaptation),
                    MakeIntegerChecker<int32_t> ())
      .AddAttribute("CongestionWindowType",
                    "Defines the CWND Type (either tcp or static)",
                    StringValue ("tcp"),
                    MakeStringAccessor (&PlayerRequester::cwnd_type),
                    MakeStringChecker ())
      .AddAttribute("AlogicType",
                    "Defines the Adaptation Logic Type (LayerdSieber or SimpleBuffer)",
                    IntegerValue (-1),
                    MakeIntegerAccessor (&PlayerRequester::alogic_type),
                    MakeIntegerChecker<int32_t> () );
  /* AddAttribute("LevelCount", "The amount of levels as a positive integer > 0",
                    IntegerValue(DEFAULT_AMOUNT_LEVELS),
                    MakeIntegerAccessor(&SVCCountingStrategy<Parent>::m_levelCount),
                             MakeIntegerChecker<int32_t>()); */
  return tid;
}

// Processing upon start of the application
void PlayerRequester::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();
  if (enableAdaptation == 0)
  {
    player = player::PlayerFactory::getInstance()->createPlayer(
          mpd_path, utils::WindowNDN, cwnd_type, static_cast<dashimpl::AdaptationLogicType> (alogic_type), this->GetNode ());
  } else if (enableAdaptation == 1){
    player = player::PlayerFactory::getInstance()->createPlayer(
          mpd_path, utils::NACKCountingSVC, cwnd_type, static_cast<dashimpl::AdaptationLogicType>(alogic_type), this->GetNode ());
  }
  else if ((enableAdaptation == 2))
  {
    player = player::PlayerFactory::getInstance()->createPlayer(
          mpd_path, utils::SVCWindowNDN, cwnd_type, static_cast<dashimpl::AdaptationLogicType>(alogic_type), this->GetNode ());
  }
  else
  {
    fprintf(stderr, "Invalid setting for player\n");
    exit(-1);
  }
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

