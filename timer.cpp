#include "timer.h"
#ifdef WIN32
#else
#include <stdint.h>
#include <sys/timerfd.h>
#include <unistd.h>
#endif

namespace utility {

#ifdef WIN32
#define INVALID_TIMER NULL
#else
#define INVALID_TIMER -1
#endif

Timer::Timer() {
  timer_ = INVALID_TIMER;
  period_ = 0;
}

Timer::~Timer() {
  Uninit();
}

bool Timer::Init(int period) {
  if (period <= 0) {
    return false;
  }
  if (timer_ != INVALID_TIMER) {
    return true;
  }
#ifdef WIN32
  timer_ = CreateWaitableTimer(NULL, FALSE, NULL);
#else
  timer_ = timerfd_create(CLOCK_REALTIME, 0);
#endif
  if (timer_ == INVALID_TIMER) {
    return false;
  }
  if (!ResetTimer(period)) {
    Uninit();
    return false;
  }
  return true;
}

#ifdef WIN32
bool Timer::ResetTimer(int period) {
  if (period < 0 || timer_ == INVALID_TIMER) {
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
#else
bool Timer::ResetTimer(int period) {
  if (period < 0 || timer_ == INVALID_TIMER) {
    return false;
  }
  period_ = period;
  itimerspec timer_spec = {0};
  timer_spec.it_value.tv_sec = period_;
  timer_spec.it_interval.tv_sec = period_;
  if (timerfd_settime(timer_, 0, &timer_spec, nullptr) == -1){
    return false;
  }
  return true;
}
#endif

#ifdef WIN32
bool Timer::Wait() {
  auto wait_result = WaitForSingleObject(timer_, INFINITE);
  if (wait_result != WAIT_FAILED) {
    return true;
  }
  return false;
}
#else
bool Timer::Wait() {
  uint64_t exp = 0;
  auto wait_result = read(timer_, &exp, sizeof(exp));
  if (wait_result > 0) {
    return true;
  }
  return false;
}
#endif

void Timer::Uninit() {
  if (timer_ != INVALID_TIMER) {
    ResetTimer(0);
#ifdef WIN32
    CloseHandle(timer_);
#else
    close(timer_);
#endif
    timer_ = INVALID_TIMER;
  }
  period_ = 0;
}

} // namespace utility