#ifndef PLAYERLEVELHISTORY_H
#define PLAYERLEVELHISTORY_H


#include <map>
#include <string>
#include <fstream>

using namespace std;

class PlayerLevelHistory
{
public:
  PlayerLevelHistory();

  void SetPlayerLevel(unsigned int segmentNumber, unsigned int level, unsigned int buffer);
  void logUnsmoothSecond(unsigned int segmentNumber, unsigned int duration);
  bool WriteToFile(std::string FileName);

protected:
  std::map<int, unsigned int> levelHistory;
  std::map<int, unsigned int> bufferHistory;
  std::map<unsigned int, unsigned int> unsmooth_seconds;
};

#endif // PLAYERLEVELHISTORY_H
