#ifndef CONGESTIONWINDOW_H
#define CONGESTIONWINDOW_H


#include <algorithm>

#define CONG_WINDOW_MULTIPLICATOR 2
#define CONG_WINDOW_MIN 2


namespace ns3
{
  class CongestionWindow
  {
  public:
    CongestionWindow();
    CongestionWindow(int window_size, int window_threshold);

    void SetWindowSize(int window_size);
    void SetThreshold(int window_threshold);

    int GetWindowSize();
    int GetThreshold();

    int IncreaseWindow();
    int DecreaseWindow();

  protected:
    int window_size;
    int window_threshold;
  };
}
#endif // CONGESTIONWINDOW_H
