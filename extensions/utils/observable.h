#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include "observer.h"

#include <vector>
#include <algorithm>

namespace ns3
{
  namespace utils
  {
    class Observable
    {
    public:

      bool addObserver(Observer *obs);
      bool removeObserver(Observer *obs);
      void notifyAll();

    private:
      std::vector<Observer*> observers;

    };
  }
}
#endif // OBSERVABLE_H
