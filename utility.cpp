#include "utility.h"
#include <memory>
#ifdef WIN32
#include <time.h>
#include <Windows.h>
#else
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace utility {

#ifdef WIN32
std::wstring AStringToW(const std::string& a_string, bool utf8) {
  if (a_string.empty()) {
    return L"";
  }
  std::unique_ptr<wchar_t[]> w_string;
  if (utf8) {
    auto string_size = MultiByteToWideChar(CP_UTF8, 0, a_string.c_str(), -1, NULL, 0);
    w_string.reset(new wchar_t[string_size]);
    MultiByteToWideChar(CP_UTF8, 0, a_string.c_str(), -1, w_string.get(), string_size);
  } else {
    auto string_size = MultiByteToWideChar(CP_ACP, 0, a_string.c_str(), -1, NULL, 0);
    w_string.reset(new wchar_t[string_size]);
    MultiByteToWideChar(CP_ACP, 0, a_string.c_str(), -1, w_string.get(), string_size);
  }
  return w_string.get();
}
#else
std::wstring AStringToW(const std::string& a_string, bool utf8) {
  if (a_string.empty()) {
    return L"";
  }
  auto string_size = mbstowcs(nullptr, a_string.c_str(), 0);
  ++string_size;
  std::unique_ptr<wchar_t[]> w_string(new wchar_t[string_size]);
  mbstowcs(w_string.get(), a_string.c_str(), string_size);
  return w_string.get();
}
#endif

#ifdef WIN32
std::string WStringToA(const std::wstring& w_string) {
  if (w_string.empty()) {
    return "";
  }
  auto string_size = WideCharToMultiByte(CP_OEMCP, 0, w_string.c_str(), -1, NULL, 0, NULL, NULL);
  std::unique_ptr<char[]> a_string(new char[string_size]);
  WideCharToMultiByte(CP_OEMCP, 0, w_string.c_str(), -1, a_string.get(), string_size, NULL, NULL);
  return a_string.get();
}
#else
std::string WStringToA(const std::wstring& w_string) {
  if (w_string.empty()) {
    return "";
}
  auto string_size = wcstombs(nullptr, w_string.c_str(), 0);
  ++string_size;
  std::unique_ptr<char[]> a_string(new char[string_size]);
  wcstombs(a_string.get(), w_string.c_str(), string_size);
  return a_string.get();
}
#endif

std::wstring GetExeName(const std::wstring& ext) {
  auto exe_path = GetExePath();
#ifdef WIN32
  auto last_slash_pos = exe_path.rfind(L'\\');
#else
  auto last_slash_pos = exe_path.rfind(L'/');
#endif
  auto dot_exe_pos = exe_path.rfind(ext);
  auto exe_name_length = dot_exe_pos - ++last_slash_pos;
  std::wstring exe_name(exe_path, last_slash_pos, exe_name_length);
  return std::move(exe_name);
}

#ifdef WIN32
std::wstring GetExePath() {
  wchar_t exe_path[MAX_PATH] = {0};
  GetModuleFileName(NULL, exe_path, _countof(exe_path));
  return exe_path;
}
#else
std::wstring GetExePath() {
  char exe_path[1024] = {0};
  readlink("/proc/self/exe", exe_path, _countof(exe_path));
  auto str_path = AStringToW(exe_path, false);
  return str_path;
}
#endif

std::wstring GetExeDirectory() {
  auto exe_path = GetExePath();
#ifdef WIN32
  auto last_slash_pos = exe_path.rfind(L'\\');
#else
  auto last_slash_pos = exe_path.rfind(L'/');
#endif
  std::wstring exe_dir(exe_path, 0, last_slash_pos);
  return std::move(exe_dir);
}

#ifdef WIN32
int GetProcessorNum() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}
#else
int GetProcessorNum() {
  return sysconf(_SC_NPROCESSORS_CONF);
}
#endif

void GetCurrentDayTime(DayTime& now) {
  GetSpecialDayTime(now, 0);
}

#ifdef WIN32
void GetCurrentAccurateTime(AccurateTime& now) {
  SYSTEMTIME sys_time = {0};
  GetLocalTime(&sys_time);
  now.year = sys_time.wYear;
  now.month = sys_time.wMonth;
  now.day = sys_time.wDay;
  now.hour = sys_time.wHour;
  now.minute = sys_time.wMinute;
  now.second = sys_time.wSecond;
  now.milliseconds = sys_time.wMilliseconds;
}
#else
void GetCurrentAccurateTime(AccurateTime& now) {
  timeval now_tv = {0};
  gettimeofday(&now_tv, nullptr);
  tm now_tm = {0};
  localtime_r(&now_tv.tv_sec, &now_tm);
  now.year = now_tm.tm_year + 1900;
  now.month = now_tm.tm_mon + 1;
  now.day = now_tm.tm_mday;
  now.hour = now_tm.tm_hour;
  now.minute = now_tm.tm_min;
  now.second = now_tm.tm_sec;
  now.milliseconds = now_tv.tv_usec / 1000;
}
#endif

void GetSpecialDayTime(DayTime& now, int due) {
  auto now_time = time(nullptr);
  now_time += due;
  tm now_tm = {0};
#ifdef WIN32
  localtime_s(&now_tm, &now_time);
#else
  localtime_r(&now_time, &now_tm);
#endif
  now.year = now_tm.tm_year + 1900;
  now.month = now_tm.tm_mon + 1;
  now.day = now_tm.tm_mday;
}

unsigned int GetCurrentThreadId() {
#ifdef WIN32
  return ::GetCurrentThreadId();
#else
  return syscall(__NR_gettid);
#endif
}

} // namespace utility