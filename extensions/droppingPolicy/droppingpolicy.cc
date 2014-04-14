#include "droppingpolicy.h"

DroppingPolicy::DroppingPolicy()
{
  // init metric with 0.0
  metric = 0.0;
}


void DroppingPolicy::SetMetric(double metric)
{
    if (metric < 0.0)
        return;

    if (metric > 1.0)
        return;

    this->metric = metric;
}


void DroppingPolicy::Feed(LevelStatistics& L)
{
    this->dropProbs.resize(L.GetAmountOfLevels(), 0.0);

    // calculate dropping probabilities
    CalculateDroppingProbabilities(L);
}


double DroppingPolicy::GetDroppingProbability(unsigned int layer)
{
    return this->dropProbs[layer];
}




void DroppingPolicy::Print(ostream& os)
{
    os << "DropProb: ";
    for (std::vector<double>::iterator it = this->dropProbs.begin();
         it != this->dropProbs.end();
         ++it)
    {
        double prob = *it;
        os << prob << ", ";
    }

    os << endl;
}
