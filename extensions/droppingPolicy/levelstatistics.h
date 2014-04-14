#ifndef LEVELSTATISTICS_H
#define LEVELSTATISTICS_H


#include <iostream>
#include <vector>

using namespace std;


/*! LevelStatistics class which collects the amount of levels in a vector */
class LevelStatistics
{
public:
    /** c-tor - does nothing */
    LevelStatistics();

    /*!
      Returns the amount of levels that are available for counting purpose
      */
    unsigned int GetAmountOfLevels();

    /*!
      Returns the sum over all available levels
      \return sum of levels
      */
    unsigned int GetTotalLevelCounter();


    /*!
      Returns the sum over all levels 0 to level
      \param level
      \return sum of levels between 0 and level
      */
    unsigned int GetTotalLevelCounter(unsigned int level);

    /*!
      Checks if a certain level is available in the vector array
      \return true if level exists (level < numLevels)
      */
    bool HasLevel(unsigned int level);

    /*!
      Creates Levels in the vector array; This method might overwrite the content
      of the vector.
      \param numLevels the amount of levels to be created, from 0 to numLevels-1
      */
    bool CreateLevels(unsigned int numLevels);

    /*!
      Increases the level counter by 1 for a certain level
      \param level the level to be increased by 1
      \return the updated counter value of the level
      */
    unsigned int IncreaseLevelCounter(unsigned int level);

    /*!
      Decrease the level counter by 1, if possible, for a certain level.
      The level counter may not drop below 0.
      \param level the level to be decreased by 1
      \return the updated coutner value of the level
      */
    unsigned int DecreaseLevelCounter(unsigned int level);

    /*!
      Requests the level counter of a certain level
      \param level the level to be requested
      \return the counter value of the level
      */
    unsigned int GetLevelCounter(unsigned int level);


    /*!
      Requests the level probability of a certain level
      \param level the level to be requested
      \return the probabiliy value of the level - only updated when RefreshStatistics() is called
      */
    double GetLevelProbability(unsigned int level);


    /*!
      Prints the current data in the levelCounting and levelProbs vectors to the output stream
      \param ostream the output stream data will be forwarded to
      */
    void Print(ostream& s);


    /*!
      Resets the counters to 0, but does not touch the probabilities/statistics
      */
    void ResetCounters();

    /*!
      Refreshes the statistics/probabilities, but does not touch the counters.
      Important: This method normalizes the vector so that sum(levelProbs) = 1
      */
    void RefreshStatistics();

protected:
    std::vector<unsigned int> levelCounting;
    std::vector<double>       levelProbs;
};

#endif // LEVELSTATISTICS_H
