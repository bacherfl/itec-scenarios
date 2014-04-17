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

  if (levelHistory.find (segmentNumber) != levelHistory.end () || levelHistory[segmentNumber] < level) //write biggest level
    this->levelHistory[segmentNumber] = level;

  if (bufferHistory.find (segmentNumber) != bufferHistory.end () || bufferHistory[segmentNumber] > buffer) //write smaller buffer
    this->bufferHistory[segmentNumber] = buffer;
}

void PlayerLevelHistory::logUnsmoothSecond (unsigned int segmentNumber, unsigned int duration)
{
  if(unsmooth_seconds.find (segmentNumber) != unsmooth_seconds.end ())
    this->unsmooth_seconds[segmentNumber] += duration;
  else
    this->unsmooth_seconds[segmentNumber] = duration;
}

bool PlayerLevelHistory::WriteToFile(std::string FileName)
{
  ofstream file;
  FileName = "output/" + FileName;
  file.open(FileName.c_str());

  file << "SegmentNr, Level, Buffer, Unsmooth Second(s)" << endl;

  double avg_level = 0.0;
  double buf_fill = 0.0;
  unsigned int unsmooth = 0;

  for (int i = 0; i < this->levelHistory.size (); i++)
  {
    avg_level += levelHistory.at(i);
    buf_fill += bufferHistory.at (i);


    if(unsmooth_seconds.find (i) != unsmooth_seconds.end ())
    {
      file << i << ", " << levelHistory.at(i) << ", " << bufferHistory.at(i) << ", " << unsmooth_seconds.at (i) << endl;
      unsmooth += unsmooth_seconds.at (i);
    }
    else
      file << i << ", " << levelHistory.at(i) << ", " << bufferHistory.at(i) << ", " << (unsigned int) 0 << endl;
  }

  file << "AVG Level = " << (avg_level/levelHistory.size ()) << endl;
  file << "AVG Buffer = " << (buf_fill/levelHistory.size ()) << endl;
  file << "Unsmooth Secondsr = " << unsmooth << endl;

  file.close();

  return true;
}
