/************************************************************************/
/*  Scope Guard                                                         */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_SCOPE_GUARD_H_
#define UTILITY_SCOPE_GUARD_H_

#include "uncopyable.h"
#include <functional>

namespace utility {

class ScopeGuard : public Uncopyable {
 public:
  explicit ScopeGuard(std::function<void ()>&& on_exit_func) : on_exit_func_(std::move(on_exit_func)) {}
  ~ScopeGuard() {
    if (on_exit_func_ != nullptr) {
      on_exit_func_();
    }
  }

 private:
  std::function<void ()> on_exit_func_;
};

} // namespace utility

#endif // UTILITY_SCOPE_GUARD_H_
