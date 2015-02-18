#ifndef IDOWNLOADER_H
#define IDOWNLOADER_H

#include <stdlib.h>
#include <string>

class IDownLoader {

public:
    virtual ~IDownLoader() {}
    virtual void SendInterest(std::string name, uint32_t seqNum) = 0;
};

#endif // IDOWNLOADER_H
