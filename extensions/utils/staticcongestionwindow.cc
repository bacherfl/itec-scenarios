#include "staticcongestionwindow.h"


using namespace ns3;

StaticCongestionWindow::StaticCongestionWindow() : CongestionWindow()
{
}


int StaticCongestionWindow::IncreaseWindow()
{
  /*this->window_size = this->window_size * 2;

  if (this->window_size > this->recv_window_size)
    this->window_size = this->recv_window_size;*/

  // do nothing
  return this->window_size;
}


int StaticCongestionWindow::DecreaseWindow()
{

  // do nothing
  return this->window_size;
}
