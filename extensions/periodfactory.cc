#include "periodfactory.h"
#include "jsoncpp/json/json.h"
#include <string>
#include <sstream>
#include <fstream>

PeriodFactory* PeriodFactory::instance = NULL;

PeriodFactory::PeriodFactory()
{
}

PeriodFactory* PeriodFactory::GetInstance()
{
    if (PeriodFactory::instance == NULL) {
        PeriodFactory::instance = new PeriodFactory;
    }
    return PeriodFactory::instance;
}

std::map<int, std::vector<Period *> > PeriodFactory::GetPeriods(std::string configFileName)
{
    std::map<int, std::vector<Period *> > periodsMap;
    std::ifstream file(configFileName);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string configStr = buffer.str();

    Json::Reader reader;
    Json::Value root;

    bool parsingSuccessful = reader.parse(configStr, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << configStr <<  "\n";
        return periodsMap;
    }

    int periodLength = root["periodLength"].asInt(); //TODO enable individual lengths for periods

    Json::Value periods = root["Periods"];

    for (int i = 0; i < periods.size(); i++) {
        Json::Value period = periods[i];

        Json::Value regionStatistics = period["regionStatistics"];
        for (int j = 0; j < regionStatistics.size(); j++) {
            Period *p = new Period;
            p->length = periodLength;
            Json::Value stats = regionStatistics[j]["statistics"];

            for (int x = 0; x < stats.size(); x++) {
                std::string name = stats[x]["name"].asString();
                double popularity = stats[x]["popularity"].asDouble();
                double size = stats[x]["size"].asDouble();
                p->popularities[name] = popularity;
                p->contentSizes[name] = size;
            }
            periodsMap[regionStatistics[j]["region"].asInt()].push_back(p);
        }
    }
    return periodsMap;
}

std::vector<Period *> PeriodFactory::GetPeriodsForRegion(std::string configFileName, int regionId)
{
    std::vector<Period *> periodsVector;

    std::ifstream file(configFileName);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string configStr = buffer.str();

    Json::Reader reader;
    Json::Value root;

    bool parsingSuccessful = reader.parse(configStr, root);
    if (!parsingSuccessful) {
        std::cout << "could not parse data" << configStr <<  "\n";
        return periodsVector;
    }

    int periodLength = root["periodLength"].asInt(); //TODO enable individual lengths for periods

    Json::Value periods = root["Periods"];

    for (int i = 0; i < periods.size(); i++) {
        Json::Value period = periods[i];
        Period *p = new Period;
        p->length = periodLength;
        Json::Value regionStatistics = period["regionStatistics"];
        for (int j = 0; j < regionStatistics.size(); j++) {
            if (regionStatistics[j]["region"].asInt() == regionId) {
                Json::Value stats = regionStatistics[j]["statistics"];

                for (int x = 0; x < stats.size(); x++) {
                    std::string name = stats[x]["name"].asString();
                    double popularity = stats[x]["popularity"].asDouble();
                    double size = stats[x]["size"].asDouble();
                    p->popularities[name] = popularity;
                    p->contentSizes[name] = size;
                }
            }
        }
        periodsVector.push_back(p);
    }
    return periodsVector;
}
