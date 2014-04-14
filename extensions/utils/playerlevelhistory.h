#ifndef PLAYERLEVELHISTORY_H
#define PLAYERLEVELHISTORY_H


#include <vector>
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
  std::vector<unsigned int> levelHistory;
  std::vector<unsigned int> bufferHistory;
};

#endif // PLAYERLEVELHISTORY_H
