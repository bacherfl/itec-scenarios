#ifndef OBSERVER_H
#define OBSERVER_H

namespace ns3
{
  namespace utils
  {
    class Observer
    {
    public:
      virtual ~Observer(){}
      virtual void update() = 0;
    };
  }
}
#endif // OBSERVER_H
