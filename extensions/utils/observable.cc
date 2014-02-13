#include "observable.h"

using namespace ns3::utils;

bool Observable::addObserver(Observer *obs)
{

  std::vector<Observer*>::iterator it = find (observers.begin(), observers.end (), obs);

  if(it == observers.end ())
    observers.push_back (obs);

  return true;
}

bool Observable::removeObserver(Observer *obs)
{
  std::vector<Observer*>::iterator it = find (observers.begin(), observers.end (), obs);

  if(it != observers.end ())
    {
      observers.erase (it);
      return true;
    }
  return false;
}

void Observable::notifyAll()
{
  for(std::vector<Observer*>::iterator it = observers.begin (); it != observers.end (); ++it)
  {
    (*it)->update();
  }
}

