/************************************************************************/
/*  Uncopyable Class                                                    */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_UNCOPYABLE_H_
#define UTILITY_UNCOPYABLE_H_

namespace utility {

class Uncopyable {
 public:
  Uncopyable() = default;
  ~Uncopyable() = default;

 private:
  Uncopyable(const Uncopyable&) = delete;
  Uncopyable& operator=(const Uncopyable&) = delete;
};

} // namespace utility

#endif	// UTILITY_UNCOPYABLE_H_