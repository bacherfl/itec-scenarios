#ifndef PLAYERLEVELHISTORY_H
#define PLAYERLEVELHISTORY_H


#include <map>
#include <string>
#include <fstream>
#include <stdint.h>
#include "ns3-dev/ns3/log.h"

using namespace std;

class PlayerLevelHistory
{
public:
  PlayerLevelHistory();

  void NotifyStart(double startTime);
  void NotifyEnd(double endTime);
  void SetRequestedPlayerLevel(unsigned int segmentNumber, unsigned int requestedLevel);
  void SetConsumedPlayerLevel(unsigned int segmentNumber, unsigned int consumedLevel);
  void SetPlayerLevel(unsigned int segmentNumber, unsigned int level, unsigned int buffer, unsigned int segSize, int64_t dlDuration);
  void logUnsmoothSecond(unsigned int segmentNumber, double duration);
  void logDownloadedVideo(std::string mpd_video_name);
  bool WriteToFile(std::string FileName);

protected:
  std::map<int, unsigned int> levelHistory;
  std::map<int, unsigned int> requestedLevelHistory;
  std::map<int, unsigned int> bufferHistory;
  std::map<int, unsigned int> segSizeHistory;
  std::map<int, int64_t> dlDurationHistory;
  std::map<unsigned int, double> unsmooth_seconds;
  std::string mpd_video_name;
  double startTime;
  double endTime;
};

#endif // PLAYERLEVELHISTORY_H
