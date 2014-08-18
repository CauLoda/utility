/************************************************************************/
/*  Timer                                                               */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_TIMER_H_
#define UTILITY_TIMER_H_

#include "uncopyable.h"
#include <Windows.h>

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
  HANDLE timer_;
  int period_;
};

} // namespace utility

#endif // UTILITY_TIMER_H_