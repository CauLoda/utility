/************************************************************************/
/*  Timer                                                               */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_TIMER_H_
#define UTILITY_TIMER_H_

#include "uncopyable.h"
#ifdef WIN32
#include <Windows.h>
#else
#endif

namespace utility {

class Timer : public Uncopyable {
 public:
  Timer();
  ~Timer();

  bool Init(int period);
  bool ResetTimer(int period);
  bool Wait();
  void Uninit();

 private:
#ifdef WIN32
  HANDLE timer_;
#else
  int timer_;
#endif
  int period_;
};

} // namespace utility

#endif // UTILITY_TIMER_H_