#ifndef STATICCONGESTIONWINDOW_H
#define STATICCONGESTIONWINDOW_H

#include "congestionwindow.h"


namespace ns3
{
  class StaticCongestionWindow : public CongestionWindow
  {
  public:
    StaticCongestionWindow();

    virtual int IncreaseWindow();
    virtual int DecreaseWindow();
  };
}
#endif // STATICCONGESTIONWINDOW_H
