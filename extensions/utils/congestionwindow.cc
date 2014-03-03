#include "congestionwindow.h"

using namespace ns3;


CongestionWindow::CongestionWindow()
{
  this->window_size = CONG_WINDOW_MIN;
  this->window_threshold = 64;

  this->recv_window_size = 100;

  this->start_window_size = window_size;
  this->start_window_threshold = window_threshold;
}


CongestionWindow::CongestionWindow(int window_size, int window_threshold)
{
  this->window_size = window_size;
  this->window_threshold = window_threshold;

  this->start_window_size = window_size;
  this->start_window_threshold = window_threshold;
}

void CongestionWindow::Reset()
{
  this->window_size = this->start_window_size;
  this->window_threshold = this->start_window_threshold;
}

void CongestionWindow::SetWindowSize(int window_size)
{
  this->window_size = window_size;
  if (this->window_size < CONG_WINDOW_MIN)
      this->window_size = CONG_WINDOW_MIN;
  if (this->window_size > this->recv_window_size)
      this->window_size = this->recv_window_size;
}

void CongestionWindow::SetThreshold(int window_threshold)
{
  this->window_threshold = window_threshold;
}

int CongestionWindow::GetWindowSize()
{
  return this->window_size;
}

int CongestionWindow::GetThreshold()
{
  return this->window_threshold;
}


void CongestionWindow::SetReceiverWindowSize(int recv_window)
{
  this->recv_window_size = recv_window;
}

int CongestionWindow::GetReceiverWindowSize()
{
  return this->recv_window_size;
}

int CongestionWindow::IncreaseWindow()
{
  // congestion avoidance
  if (this->window_size * CONG_WINDOW_MULTIPLICATOR > this->window_threshold)
  {
    this->window_size = this->window_size + 1;
  }
  else // slow start
  {
    this->window_size = this->window_size * CONG_WINDOW_MULTIPLICATOR;
  }

  // make sure its not higher than the receiver window size
  if (this->window_size > this->recv_window_size)
      this->window_size = this->recv_window_size;

  // return new window size
  return this->window_size;
}


int CongestionWindow::DecreaseWindow()
{
  this->window_size = std::max<int>(this->window_size / 2, CONG_WINDOW_MIN);

  return this->window_size;
}

