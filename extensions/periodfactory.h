#ifndef PERIODFACTORY_H
#define PERIODFACTORY_H

#include "period.h"
#include <vector>

class PeriodFactory
{
public:        
    static PeriodFactory* GetInstance();
    std::map<int, std::vector<Period *> > GetPeriods(std::string configFileName);
    std::vector<Period *> GetPeriodsForRegion(std::string configFileName, int regionId);

private:
    PeriodFactory();
    static PeriodFactory *instance;
};

#endif // PERIODFACTORY_H
