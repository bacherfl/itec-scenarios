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

      virtual bool addObserver(Observer *obs);
      virtual bool removeObserver(Observer *obs);
      virtual void notifyAll(Observer::ObserverMessage msg);

    private:
      std::vector<Observer*> observers;

    };
  }
}
#endif // OBSERVABLE_H
