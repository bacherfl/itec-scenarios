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
  bool WriteToFile(std::string FileName);

protected:
  std::map<int, unsigned int> levelHistory;
  std::map<int, unsigned int> bufferHistory;
};

#endif // PLAYERLEVELHISTORY_H
