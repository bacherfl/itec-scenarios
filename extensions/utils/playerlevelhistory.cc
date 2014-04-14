#include "playerlevelhistory.h"

#include <iostream>
using namespace std;

PlayerLevelHistory::PlayerLevelHistory()
{
}


void PlayerLevelHistory::SetPlayerLevel(unsigned int segmentNumber,
                                        unsigned int level, unsigned int buffer)
{
 // cout << "SEgmentNumber: " << segmentNumber << ", level=" << level << ", size=" << levelHistory.size() << endl;
  if (segmentNumber == this->levelHistory.size())
  {
    this->levelHistory.push_back(level);
  } else {
    this->levelHistory[segmentNumber] = level;
  }

  if (segmentNumber == this->bufferHistory.size())
  {
    this->bufferHistory.push_back(buffer);
  } else {
    this->bufferHistory[segmentNumber] = buffer;
  }
}



bool PlayerLevelHistory::WriteToFile(std::string FileName)
{
  ofstream file;
  FileName = "output/" + FileName;
  file.open(FileName.c_str());

  file << "SegmentNr, Level, Buffer" << endl;

  for (int i = 0; i < this->levelHistory.size(); i++)
  {
    file << i << ", " << levelHistory.at(i) << ", " << bufferHistory.at(i) << endl;
  }

  file.close();

  return true;
}
