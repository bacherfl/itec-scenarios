#ifndef PERIOD_H
#define PERIOD_H

#include <map>
#include <string>

struct Period {
    int length;
    std::map<std::string, double> popularities;
    std::map<std::string, double> contentSizes;
};


#endif // PERIOD_H
