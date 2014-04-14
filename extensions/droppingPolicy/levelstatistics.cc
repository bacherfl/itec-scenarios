#include "levelstatistics.h"



LevelStatistics::LevelStatistics()
{
}



unsigned int LevelStatistics::GetAmountOfLevels()
{
    return this->levelCounting.size();
}

unsigned int LevelStatistics::GetTotalLevelCounter()
{
    unsigned int cnt = 0;
    for (std::vector<unsigned int>::iterator it = this->levelCounting.begin();
         it != this->levelCounting.end();
         ++it)
    {
        cnt += (*it);
    }
    return cnt;
}


bool LevelStatistics::HasLevel(unsigned int level)
{
    return level < this->levelCounting.size();
}


void LevelStatistics::Print(ostream& os)
{
    os << "cnt: ";
    for (std::vector<unsigned int>::iterator it = this->levelCounting.begin();
         it != this->levelCounting.end();
         ++it)
    {
        unsigned int level = *it;
        os << level << ", ";
    }

    os << endl << "prob: ";

    for (std::vector<double>::iterator it = this->levelProbs.begin();
         it != this->levelProbs.end();
         ++it)
    {
        double probs = *it;
        os << probs << ", ";
    }

    os << endl;
}

bool LevelStatistics::CreateLevels(unsigned int numLevels)
{
    bool okay = true;
    this->levelCounting.resize(numLevels, 0);
    this->levelProbs.resize(numLevels, 0.0);


    return okay;
}



unsigned int LevelStatistics::IncreaseLevelCounter(unsigned int level)
{
    this->levelCounting[level] = this->levelCounting[level] + 1;
    return this->levelCounting[level];
}


unsigned int LevelStatistics::DecreaseLevelCounter(unsigned int level)
{
    if (this->levelCounting[level] != 0)
        this->levelCounting[level] = this->levelCounting[level] - 1;

    return this->levelCounting[level];
}


unsigned int LevelStatistics::GetLevelCounter(unsigned int level)
{
    return this->levelCounting[level];
}


unsigned int LevelStatistics::GetTotalLevelCounter(unsigned int level)
{
    double sum = 0.0;
    unsigned int numLevels = level;
    if (numLevels >= this->levelCounting.size() && numLevels != 0 && this->levelCounting.size() != 0)
        numLevels = this->levelCounting.size() - 1;



    for (int i = 0; i <= numLevels; i++)
    {
        sum = sum + this->levelCounting[i];
    }

    return sum;
}



double LevelStatistics::GetLevelProbability(unsigned int level)
{
    return this->levelProbs[level];
}


void LevelStatistics::ResetCounters()
{
    unsigned int numLevels = this->levelCounting.size();

    for (int i = 0; i < numLevels; i++)
    {
        this->levelCounting[i] = 0;
    }
}



void LevelStatistics::RefreshStatistics()
{
    unsigned int cnt = GetTotalLevelCounter();

    for (int i = 0; i < this->GetAmountOfLevels(); i++)
    {
        this->levelProbs[i] = (double)this->levelCounting[i] / (double)cnt;
    }
}
