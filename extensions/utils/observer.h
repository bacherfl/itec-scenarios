#ifndef OBSERVER_H
#define OBSERVER_H

namespace ns3
{
  namespace utils
  {
    class Observer
    {

    public:

      enum ObserverMessage
      {
          No_Message,
          SegmentReceived,
          NackReceived,
          Segment95PercentReceived
      };

      virtual ~Observer(){}
      virtual void update(ObserverMessage msg) = 0;
    };
  }
}
#endif // OBSERVER_H
