#include "log.h"
#include "singleton.h"
#include "timer.h"
#include "uncopyable.h"
#include "utility.h"
#include <algorithm>
#include <fstream>
#include <memory>
#include <thread>
#include <vector>
#ifdef WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace utility {

class Logger : public Uncopyable {
 public:
  Logger();
  ~Logger();
  void InitLog(const char* init_info, int log_level);
  bool Logging(LogLevel log_level, const char* file_name, int line_number, const char* function_name, const char* format_str, va_list args);

 private:
  bool Logging(const char* log_data, bool is_new_day);
  const char* GetLevelString(LogLevel level);
  bool InitClear();
  bool LoopClear();
  bool ClearOldLog();

 private:
#ifdef WIN32
  std::wstring file_pre_path_;
  std::wstring thisday_file_name_;
#else
  std::string file_pre_path_;
  std::string thisday_file_name_;
#endif
  std::ofstream log_file_;
  std::mutex file_lock_;
  int log_level_;
  Timer clear_timer_;
  std::unique_ptr<std::thread> clear_thread_;
};

Logger::Logger() {
  log_level_ = kStartup | kShutdown | kInfo | kWarning | kError;
}

Logger::~Logger() {
  file_lock_.lock();
  if (log_file_.is_open()) {
    log_file_.close();
  }
  file_lock_.unlock();
  clear_timer_.Uninit();
  if (clear_thread_ != nullptr) {
    clear_thread_->join();
    clear_thread_ = nullptr;
  }
}

#ifdef WIN32
void Logger::InitLog(const char* init_info, int log_level) {
  log_level_ = log_level;
  auto exe_dir = GetExeDirectory();
  auto log_dir = exe_dir + L"\\log";
  CreateDirectory(log_dir.c_str(), NULL);
  auto exe_name = GetExeName(L".exe");
  file_pre_path_ = log_dir + L"\\" + exe_name + L"_";
  InitClear();
  DayTime now;
  GetCurrentDayTime(now);
  wchar_t today_file_name[1024] = {0};
  swprintf_s(today_file_name, _countof(today_file_name), L"%04u%02u%02u.log", now.year, now.month, now.day);
  thisday_file_name_ = today_file_name;
  Logging(init_info, true);
}
#else
void Logger::InitLog(const char* init_info, int log_level) {
  log_level_ = log_level;
  auto exe_dir = GetExeDirectory();
  auto log_dir = WStringToA(exe_dir) + "/log";
  mkdir(log_dir.c_str(), 0777);
  auto exe_name = GetExeName(L".exe");
  file_pre_path_ = log_dir + "/" + WStringToA(exe_name) + "_";
  InitClear();
  DayTime now;
  GetCurrentDayTime(now);
  char today_file_name[1024] = {0};
  sprintf_s(today_file_name, _countof(today_file_name), "%04u%02u%02u.log", now.year, now.month, now.day);
  thisday_file_name_ = today_file_name;
  Logging(init_info, true);
}
#endif

bool Logger::Logging(LogLevel log_level, const char* file_name, int line_number, const char* function_name, const char* format_str, va_list args) {
  if ((log_level_ & log_level) == 0) {
    return true;
  }
  AccurateTime now;
  GetCurrentAccurateTime(now);
  char time_data[1024] = {0};
  char thread_data[1024] = {0};
  char source_data[1024] = {0};
  char level_data[1024] = {0};
  char function_data[1024] = {0};
  char format_data[1024] = {0};
  char log_data[2048] = {0};
  sprintf_s(time_data, _countof(time_data), "[%02u:%02u:%02u.%03u]|", now.hour, now.minute, now.second, now.milliseconds);
  sprintf_s(thread_data, _countof(thread_data), "[%04x]|", GetCurrentThreadId());
  //sprintf_s(source_data, _countof(source_data), "[%s:%d]|", file_name, line_number);
  sprintf_s(level_data, _countof(level_data), "[%s]|", GetLevelString(log_level));
  //sprintf_s(function_data, _countof(function_data), "[%s]|", function_name);
  _vsnprintf_s(format_data, _countof(format_data), format_str, args);
  sprintf_s(log_data, _countof(log_data), "%s%s%s%s%s%s\n", time_data, thread_data, source_data, level_data, function_data, format_data);
#ifdef WIN32
  wchar_t today_file_name[1024] = {0};
  swprintf_s(today_file_name, _countof(today_file_name), L"%04u%02u%02u.log", now.year, now.month, now.day);
#else
  char today_file_name[1024] = {0};
  sprintf_s(today_file_name, _countof(today_file_name), "%04u%02u%02u.log", now.year, now.month, now.day);
#endif
  auto is_new_day = false;
  if (thisday_file_name_ != today_file_name) {
    thisday_file_name_ = today_file_name;
    is_new_day = true;
  }
  auto result = Logging(log_data, is_new_day);
  return result;
}

bool Logger::Logging(const char* log_data, bool is_new_day) {
  std::lock_guard<std::mutex> lock(file_lock_);
  printf("%s", log_data);
  if (is_new_day) {
    if (log_file_.is_open()) {
      log_file_.close();
    }
    auto log_file_path = file_pre_path_ + thisday_file_name_;
    log_file_.open(log_file_path, std::ios::app);
    if (!log_file_.good()) {
      return false;
    }
  }
  if (!log_file_.is_open()) {
    return false;
  }
  log_file_.write(log_data, strlen(log_data));
  if (!log_file_.good()) {
    return false;
  }
  log_file_.flush();
  if (!log_file_.good()) {
    return false;
  }
  return true;
}

const char* Logger::GetLevelString(LogLevel level) {
  static const char* log_levels[] = {"Startup", "Shutdown", "Info", "Warning", "Error", "Unkown"};
  switch (level) {
  case kStartup:
    return log_levels[0];
  case kShutdown:
    return log_levels[1];
  case kInfo:
    return log_levels[2];
  case kWarning:
    return log_levels[3];
  case kError:
    return log_levels[4];
  }
  return log_levels[5];
}

bool Logger::InitClear() {
  ClearOldLog();
  AccurateTime now;
  GetCurrentAccurateTime(now);
  auto first_clear_hour = 23 - now.hour;
  auto first_clear_minute = 59 - now.minute;
  auto first_clear_second = 60 - now.second;
  auto first_clear = first_clear_hour * 24 + first_clear_minute * 60 + first_clear_second;
  auto init_result = clear_timer_.Init(first_clear);
  auto thread_proc = std::bind(&Logger::LoopClear, this);
  clear_thread_.reset(new std::thread(thread_proc));
  return init_result;
}

bool Logger::LoopClear() {
  auto first_clear = true;
  while (true) {
    clear_timer_.Wait();
    if (first_clear) {
      const auto one_day_seconds = 24 * 60 * 60;
      clear_timer_.ResetTimer(one_day_seconds);
      first_clear = false;
    }
    ClearOldLog();
  }
  return true;
}

#ifdef WIN32
bool Logger::ClearOldLog() {
  std::vector<std::wstring> all_log_file;
  WIN32_FIND_DATA find_data = {0};
  auto find_string = file_pre_path_ + L"*.log";
  auto find_handler = FindFirstFile(find_string.c_str(), &find_data);
  if (find_handler == INVALID_HANDLE_VALUE) {
    return false;
  }
  auto last_slash_pos = file_pre_path_.rfind(L'\\');
  std::wstring log_dir(file_pre_path_, 0, ++last_slash_pos);
  do {
    auto find_file = log_dir + find_data.cFileName;
    std::for_each(find_file.begin(), find_file.end(), towlower);
    all_log_file.push_back(std::move(find_file));
  } while (FindNextFile(find_handler, &find_data));
  FindClose(find_handler);
  DayTime past;
  auto past_due = -30 * 24 * 60 * 60; // push back 30 days
  GetSpecialDayTime(past, past_due);
  wchar_t base[1024] = {0};
  swprintf_s(base, _countof(base), L"%s%04u%02u%02u.log", file_pre_path_.c_str(), past.year, past.month, past.day);
  std::wstring delete_base(base);
  std::for_each(delete_base.begin(), delete_base.end(), towlower);
  for (const auto& i : all_log_file) {
    if (i < delete_base) {
      DeleteFile(i.c_str());
    }
  }
  return true;
}
#else
bool Logger::ClearOldLog() {
  std::vector<std::string> all_log_file;
  auto last_slash_pos = file_pre_path_.rfind('/');
  std::string log_dir(file_pre_path_, 0, last_slash_pos);
  auto dir = opendir(log_dir.c_str());
  if (dir == nullptr) {
    return false;
  }
  dirent* find_data = nullptr;
  while ((find_data = readdir(dir)) != nullptr) {
    auto find_file = log_dir + "/" + find_data->d_name;
    auto find_ext = find_file.find(".log");
    if (find_ext == std::string::npos) {
      continue;
    }
    std::for_each(find_file.begin(), find_file.end(), towlower);
    all_log_file.push_back(std::move(find_file));
  }
  closedir(dir);
  DayTime past;
  auto past_due = -30 * 24 * 60 * 60; // push back 30 days
  GetSpecialDayTime(past, past_due);
  char base[1024] = {0};
  sprintf_s(base, _countof(base), "%s%04u%02u%02u.log", file_pre_path_.c_str(), past.year, past.month, past.day);
  std::string delete_base(base);
  std::for_each(delete_base.begin(), delete_base.end(), towlower);
  for (const auto& i : all_log_file) {
    if (i < delete_base) {
      unlink(i.c_str());
    }
  }
  return true;
}
#endif

typedef Singleton<Logger> SingleLogger;

} // namespace utility

void InitLog(const char* init_info, int log_level) {
  auto logger = utility::SingleLogger::GetInstance();
  logger->InitLog(init_info, log_level);
}

void logging(LogLevel level,
  const char* file_name,
  int line,
  const char* func_name,
  const char* format_str,
  ...) {
  va_list args;
  va_start(args, format_str);
  auto logger = utility::SingleLogger::GetInstance();
  logger->Logging(level, file_name, line, func_name, format_str, args);
  va_end(args);
}