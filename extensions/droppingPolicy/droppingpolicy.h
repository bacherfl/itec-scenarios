#ifndef DROPPINGPOLICY_H
#define DROPPINGPOLICY_H


#include <iostream>
#include "levelstatistics.h"

using namespace std;

/*! DroppingPolicy base class which calculates a dropping policy */
class DroppingPolicy
{
public:
    DroppingPolicy();

    /*!
      Feeds LevelStatistics to the policy
      \param L the LevelStatistics class (reference)
      */
    void Feed(LevelStatistics& L);

    /*!
      Requests the dropping probability for a specified layer
      \param layer the layer to be requested
      \return the dropping probability of the layer
      */
    double GetDroppingProbability(unsigned int layer);

    /*!
      Set the metric between 0 and 1.
      \param metric the metric between 0 and 1
      */
    void SetMetric(double metric);


    /*!
      Prints the current data in the dropProbs vector to the output stream
      \param ostream the output stream data will be forwarded to
      */
    void Print(ostream& os);


protected:
    /*!
      Calculates the dropping probability - protected virtual method, needs
      to be overwritten by the extending class; Called by Feed()
      \see Feed
      */
    virtual void CalculateDroppingProbabilities(LevelStatistics& L) = 0;

    /*!
      the vector containing the dropping probabilities,
      \see GetDroppingProbability()
      */
    std::vector<double> dropProbs;

    /*!
      the metric between 0 and 1
      */
    double metric;
};

#endif // DROPPINGPOLICY_H
