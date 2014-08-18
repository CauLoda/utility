#include "timer.h"

namespace utility {

Timer::Timer() {
  timer_ = NULL;
  period_ = 0;
}

Timer::~Timer() {
  Uninit();
}

bool Timer::Init(int period) {
  if (period <= 0) {
    return false;
  }
  if (timer_ != NULL) {
    return true;
  }
  timer_ = CreateWaitableTimer(NULL, FALSE, NULL);
  if (timer_ == NULL) {
    return false;
  }
  if (!ResetTimer(period)) {
    Uninit();
    return false;
  }
  return true;
}

bool Timer::ResetTimer(int period) {
  if (period <= 0 || timer_ == NULL) {
    return false;
  }
  period_ = period;
  LARGE_INTEGER li = {0};
  const LONGLONG llTimerUnitsPerSecond = 10000000;	// 一秒对应的100个纳秒数：1后面7个0
  li.QuadPart = -(period_ * llTimerUnitsPerSecond);
  if (!SetWaitableTimer(timer_, &li, period_ * 1000, NULL, NULL, FALSE)){
    return false;
  }
  return true;
}

bool Timer::Wait() {
  auto wait_result = WaitForSingleObject(timer_, INFINITE);
  if (wait_result != WAIT_FAILED) {
    return true;
  }
  return false;
}

void Timer::Uninit() {
  if (timer_ != NULL) {
    CloseHandle(timer_);
    timer_ = NULL;
  }
  period_ = 0;
}

} // namespace utility