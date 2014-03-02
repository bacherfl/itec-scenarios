#include "congestionwindow.h"

using namespace ns3;


CongestionWindow::CongestionWindow()
{
  this->window_size = CONG_WINDOW_MIN;
  this->window_threshold = 64;
}


CongestionWindow::CongestionWindow(int window_size, int window_threshold)
{
  this->window_size = window_size;
  this->window_threshold = window_threshold;
}

void CongestionWindow::SetWindowSize(int window_size)
{
  this->window_size = window_size;
  if (this->window_size < CONG_WINDOW_MIN)
      this->window_size = CONG_WINDOW_MIN;
}

void CongestionWindow::SetThreshold(int window_threshold)
{
  this->window_threshold = window_threshold;
}

int CongestionWindow::GetWindowSize()
{
  return this->window_threshold;
}

int CongestionWindow::GetThreshold()
{
  return this->window_threshold;
}

int CongestionWindow::IncreaseWindow()
{
  // congestion avoidance
  if (this->window_size * CONG_WINDOW_MULTIPLICATOR > this->window_threshold)
  {
    this->window_size = this->window_size + 1;
  } else { // slow start
    this->window_size = this->window_size * CONG_WINDOW_MULTIPLICATOR;
  }

  // return new window size
  return this->window_size;
}


int CongestionWindow::DecreaseWindow()
{
  this->window_size = std::max<int>(this->window_size / 2, CONG_WINDOW_MIN);

  return this->window_size;
}

