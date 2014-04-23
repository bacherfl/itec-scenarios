#include "playerlevelhistory.h"

#include <iostream>
using namespace std;

PlayerLevelHistory::PlayerLevelHistory()
{
}


void PlayerLevelHistory::SetPlayerLevel(unsigned int segmentNumber,
                                        unsigned int level, unsigned int buffer, unsigned int segSize, int64_t dlDuration)
{
 // cout << "SEgmentNumber: " << segmentNumber << ", level=" << level << ", size=" << levelHistory.size() << endl;

  if (levelHistory.find (segmentNumber) != levelHistory.end () || levelHistory[segmentNumber] < level) //write biggest level
    this->levelHistory[segmentNumber] = level;

  if (bufferHistory.find (segmentNumber) == bufferHistory.end () ||
      (bufferHistory.find (segmentNumber) != bufferHistory.end () && bufferHistory[segmentNumber] > buffer) ) //write smaller buffer
    this->bufferHistory[segmentNumber] = buffer;

  if(segSizeHistory.find (segmentNumber) != segSizeHistory.end ())
    segSizeHistory[segmentNumber] += segSize;
  else
    segSizeHistory[segmentNumber] = segSize;

  if(dlDurationHistory.find (segmentNumber) != dlDurationHistory.end () && dlDurationHistory[segmentNumber] < dlDuration)
    dlDurationHistory[segmentNumber] += dlDuration;
  else
    dlDurationHistory[segmentNumber] = dlDuration;
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

  file << "SegmentNr, Level, Buffer, Unsmooth Second(s), Segment(s) Size (bytes), Download Time (ms), Goodput (kbit/s)" << endl;

  double avg_level = 0.0;
  double buf_fill = 0.0;
  double avg_bandwith = 0.0;
  unsigned int unsmooth = 0;

  int dlSpeed = 0;

  for (int i = 0; i < this->levelHistory.size (); i++)
  {
    avg_level += levelHistory.at(i);
    buf_fill += bufferHistory.at (i);

    file << i << ", ";
    file << levelHistory.at(i) << ", ";
    file << bufferHistory.at(i) << ", ";

    if(unsmooth_seconds.find (i) != unsmooth_seconds.end ())
    {
      file << unsmooth_seconds.at (i) << ", ";
      unsmooth += unsmooth_seconds.at (i);
    }
    else
      file << (unsigned int) 0 << ", ";

    file << segSizeHistory.at (i) << ", ";
    file << dlDurationHistory.at (i) << ", ";

    dlSpeed = (segSizeHistory.at(i) * 8) / dlDurationHistory.at (i); // bits * 8 / ms = kbits/s
    avg_bandwith += dlSpeed;

    file << dlSpeed;

    file << endl;
  }

  file << "AVG Level = " << (avg_level/levelHistory.size ()) << endl;
  file << "AVG Buffer size (seconds) = " << (buf_fill/bufferHistory.size ()) << endl;
  file << "Unsmooth Seconds = " << unsmooth << endl;
  file << "AVG Goodput (kbit/s)" << avg_bandwith / dlDurationHistory.size () << endl; //bits/s / 1000 = kbits / s.

  file.close();

  return true;
}
