/************************************************************************/
/*  Global Functions                                                    */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_UTILITY_H_
#define UTILITY_UTILITY_H_

#include <string>

namespace utility {

#ifndef WIN32
template <typename _CountofType, size_t _SizeOfArray>
char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)

#define sprintf_s snprintf
#define _vsnprintf_s vsnprintf
#endif

// unified time format
struct DayTime {
  unsigned short year;
  unsigned short month;
  unsigned short day;
};

struct AccurateTime : public DayTime {
  unsigned short hour;
  unsigned short minute;
  unsigned short second;
  unsigned short milliseconds;
};

// Convert ASCII string to wide string, consider special format: UTF-8
std::wstring AStringToW(const std::string& a_string, bool utf8);

// Convert wide string to ASCII string
std::string WStringToA(const std::wstring& w_string);

// Get the current executable file name, without extension
std::wstring GetExeName(const std::wstring& ext);

// Get the current executable file folder path, without last slash
std::wstring GetExeDirectory();

// Get the current executable file full path, include extension
std::wstring GetExePath();

// Get the number of current computer processor
int GetProcessorNum();

// Get now day time
void GetCurrentDayTime(DayTime& now);

// Get now accurate time
void GetCurrentAccurateTime(AccurateTime& now);

// Get special day time
void GetSpecialDayTime(DayTime& now, int due);

// Get current thread id
unsigned int GetCurrentThreadId();

} // namespace utility

#endif // UTILITY_UTILITY_H_