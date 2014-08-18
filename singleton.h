/************************************************************************/
/*  Singleton Pattern                                                   */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_SINGLETON_H_
#define UTILITY_SINGLETON_H_

#include <mutex>

namespace utility {

template <typename T>
class Singleton {
 public:
  static T* GetInstance() {
    std::lock_guard<std::mutex> lock(lock_);
    if (instance_ == nullptr) {
      instance_ = new T;
    }
    return instance_;
  }
  static void Release() {
    std::lock_guard<std::mutex> lock(lock_);
    if (instance_ != nullptr) {
      delete instance_;
      instance_ = nullptr;
    }
  }

 private:
  Singleton() = delete;

 private:
  static T* instance_;
  static std::mutex lock_;
};

template <typename T>
T* Singleton<T>::instance_ = nullptr;

template <typename T>
std::mutex Singleton<T>::lock_;

} // namespace utility

#endif	// UTILITY_SINGLETON_H_