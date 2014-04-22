#ifndef PLAYERLEVELHISTORY_H
#define PLAYERLEVELHISTORY_H


#include <map>
#include <string>
#include <fstream>
#include <stdint.h>

using namespace std;

class PlayerLevelHistory
{
public:
  PlayerLevelHistory();

  void SetPlayerLevel(unsigned int segmentNumber, unsigned int level, unsigned int buffer, unsigned int segSize, int64_t dlDuration);
  void logUnsmoothSecond(unsigned int segmentNumber, unsigned int duration);
  bool WriteToFile(std::string FileName);

protected:
  std::map<int, unsigned int> levelHistory;
  std::map<int, unsigned int> bufferHistory;
  std::map<int, unsigned int> segSizeHistory;
  std::map<int, int64_t> dlDurationHistory;
  std::map<unsigned int, unsigned int> unsmooth_seconds;
};

#endif // PLAYERLEVELHISTORY_H
