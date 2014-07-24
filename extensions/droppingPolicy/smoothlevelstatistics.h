#ifndef SMOOTHLEVELSTATISTICS_H
#define SMOOTHLEVELSTATISTICS_H

#include "levelstatistics.h"

#define SMOOTH_ALPHA_DEFAULT 0.1
#define SMOOTH_ALPHA_MIN 0.0
#define SMOOTH_ALPHA_MAX 1.0

/*! class SmoothLevelStatistics extends LevelStatistics with
  a weighted average smoothing with parameter alpha. */
class SmoothLevelStatistics : public LevelStatistics
{
public:
    SmoothLevelStatistics();

    /*! Set Parameter alpha
      \param alpha alpha must be between 0 and 1 */
    void SetAlpha(double alpha);

    /*! Get the parameter alpha */
    double GetAlpha();

    /*! Overwritten method from LevelStatistics;
      calculates smooth weighted average */
    void RefreshStatistics();

protected:
    /*! parameter alpha for weighted average;
      \see SetAlpha
      \see GetAlpha
      */
    double alpha;
};

#endif // SMOOTHLEVELSTATISTICS_H
