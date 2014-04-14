#include "smoothlevelstatistics.h"

SmoothLevelStatistics::SmoothLevelStatistics()
{
    this->alpha = SMOOTH_ALPHA_DEFAULT;
}

void SmoothLevelStatistics::SetAlpha(double alpha)
{
    if (alpha > SMOOTH_ALPHA_MAX || alpha < SMOOTH_ALPHA_MIN)
        return;

    this->alpha = alpha;
}

double SmoothLevelStatistics::GetAlpha()
{
    return this->alpha;
}


void SmoothLevelStatistics::RefreshStatistics()
{
    unsigned int cnt = GetTotalLevelCounter();
    if (cnt == 0) // do not attempt to divide through 0
        return;

    double p_new = 0.0;
    double sum = 0.0;

    for (int i = 0; i < this->GetAmountOfLevels(); i++)
    {
        p_new = (double)(this->levelCounting[i]) / (double)cnt;

        // weighted average update
        this->levelProbs[i] = alpha * p_new + (1 - alpha) * this->levelProbs[i];

        // add to sum
        sum = sum + this->levelProbs[i];

        //cout << "p_new=" << p_new << ", sum=" << sum << ", prob(l)=" << levelProbs[i] << endl;
    }

    // normalize the vector so that sum(levelProbs) = 1
    for (int i = 0; i < this->GetAmountOfLevels(); i++)
    {
        this->levelProbs[i] = this->levelProbs[i] / sum;
    }
}
