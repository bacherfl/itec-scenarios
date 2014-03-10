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

    void Reset();

    void SetWindowSize(int window_size);
    void SetThreshold(int window_threshold);

    int GetWindowSize();
    int GetThreshold();

    virtual int IncreaseWindow();
    virtual int DecreaseWindow();

    void SetReceiverWindowSize(int recv_window);
    int GetReceiverWindowSize();

  protected:
    int window_size;
    int window_threshold;

    int recv_window_size;

    int start_window_size;
    int start_window_threshold;
  };
}
#endif // CONGESTIONWINDOW_H
