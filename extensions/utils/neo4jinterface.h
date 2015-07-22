#ifndef NEO4JINTERFACE_H
#define NEO4JINTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <iostream>
#include <sstream>

class Neo4jInterface
{
public:
    Neo4jInterface();
    std::string PerformNeo4jTrx(std::string requestContent);
    std::string PerformNeo4jTrx(std::vector<std::string> statements);

    static size_t curlCallback(void *ptr, size_t size, size_t nmemb, void *stream);

private:
    CURL *ch;
    static std::stringstream recv_data;
};

#endif // NEO4JINTERFACE_H
