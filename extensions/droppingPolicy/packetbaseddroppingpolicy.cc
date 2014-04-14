#include "packetbaseddroppingpolicy.h"

PacketBasedDroppingPolicy::PacketBasedDroppingPolicy()
{
}



void PacketBasedDroppingPolicy::CalculateDroppingProbabilities(LevelStatistics& L)
{
    // call simplex
    // or call algorithm
    int cnt = L.GetAmountOfLevels();

    double sum = 0.0;

    int l = cnt-1;
    // set s_l to 1 until sum >= metric
    do
    {
        this->dropProbs[l] = 1.0;

        sum = sum + L.GetLevelProbability(l);
        l--;
    }
    while (sum < metric && l >= 0);

    cout << "Current Metric: " << metric << ", Current Sum: " << sum << ", l=" << l << endl;

    // calculate s_l for the last l
    l = l + 1;

    if (L.GetLevelProbability(l) != 0)
    {
        this->dropProbs[l] = (metric - sum) / L.GetLevelProbability(l) + 1;
    } else {
        this->dropProbs[l] = 1;
    }
    // set the remaining levels to 0
    for (int i = 0; i < l; i++)
    {
        this->dropProbs[i] = 0.0;
    }
}
