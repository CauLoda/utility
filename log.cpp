#include "log.h"
#include "singleton.h"
#include "uncopyable.h"
#include "utility.h"
#include <algorithm>
#include <fstream>
#include <vector>
#include <Windows.h>

namespace utility {

class Logger : public Uncopyable {
 public:
  Logger();
  ~Logger();
  void InitLog(const char* init_info, int log_level);
  bool Logging(LogLevel log_level, const char* file_name, int line_number, const char* function_name, const char* format_str, va_list args);

 private:
  bool LoggingToFile(const char* log_data, bool is_new_day);
  const char* GetLevelString(LogLevel level);
  bool ClearOldLog();

 private:
  std::wstring file_pre_path_;
  std::wstring thisday_file_name_;
  std::ofstream log_file_;
  std::mutex file_lock_;
  int log_level_;
};

Logger::Logger() {
  auto exe_dir = utility::GetExeDirectory();
  auto log_dir = exe_dir + L"\\log";
  CreateDirectory(log_dir.c_str(), NULL);
  auto exe_name = utility::GetExeName();
  file_pre_path_ = log_dir + L"\\" + exe_name + L"_";
  log_level_ = kStartup | kShutdown | kInfo | kWarning | kError;
}

Logger::~Logger() {
  std::lock_guard<std::mutex> lock(file_lock_);
  if (log_file_.is_open()) {
    log_file_.close();
  }
}

void Logger::InitLog(const char* init_info, int log_level) {
  log_level_ = log_level;
  SYSTEMTIME now = {0};
  GetLocalTime(&now);
  wchar_t today_file_name[MAX_PATH] = {0};
  swprintf_s(today_file_name, _countof(today_file_name), L"%04u%02u%02u.log", now.wYear, now.wMonth, now.wDay);
  thisday_file_name_ = today_file_name;
  printf("%s", init_info);
  LoggingToFile(init_info, true);
}

bool Logger::Logging(LogLevel log_level, const char* file_name, int line_number, const char* function_name, const char* format_str, va_list args) {
  if ((log_level_ & log_level) == 0) {
    return true;
  }
  SYSTEMTIME now = {0};
  char time_data[1024] = {0};
  char thread_data[1024] = {0};
  char source_data[1024] = {0};
  char level_data[1024] = {0};
  char function_data[1024] = {0};
  char format_data[1024] = {0};
  char log_data[2048] = {0};
  GetLocalTime(&now);
  sprintf_s(time_data, _countof(time_data), "[%02u:%02u:%02u.%03u]|", now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
  sprintf_s(thread_data, _countof(thread_data), "[%04x]|", GetCurrentThreadId());
  //sprintf_s(source_data, _countof(source_data), "[%s:%d]|", file_name, line_number);
  sprintf_s(level_data, _countof(level_data), "[%s]|", GetLevelString(log_level));
  //sprintf_s(function_data, _countof(function_data), "[%s]|", function_name);
  _vsnprintf_s(format_data, _countof(format_data), format_str, args);
  sprintf_s(log_data, _countof(log_data), "%s%s%s%s%s%s\n", time_data, thread_data, source_data, level_data, function_data, format_data);
  printf("%s", log_data);
  wchar_t today_file_name[MAX_PATH] = {0};
  swprintf_s(today_file_name, _countof(today_file_name), L"%04u%02u%02u.log", now.wYear, now.wMonth, now.wDay);
  auto is_new_day = false;
  if (thisday_file_name_ != today_file_name) {
    thisday_file_name_ = today_file_name;
    is_new_day = true;
  }
  return LoggingToFile(log_data, is_new_day);
}

bool Logger::LoggingToFile(const char* log_data, bool is_new_day) {
  std::lock_guard<std::mutex> lock(file_lock_);
  if (is_new_day) {
    if (log_file_.is_open()) {
      log_file_.close();
    }
    ClearOldLog();
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
  static const char* log_levels[] = { "Startup", "Shutdown", "Info", "Warning", "Error", "Unkown" };
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

bool Logger::ClearOldLog() {
  std::vector<std::wstring> all_log_file;
  _wfinddata_t find_data = {0};
  auto find_string = file_pre_path_ + L"*.log";
  auto find_handler = _wfindfirst(find_string.c_str(), &find_data);
  if (find_handler == -1) {
    return false;
  }
  auto last_slash_pos = file_pre_path_.rfind(L'\\');
  std::wstring log_dir(file_pre_path_, 0, ++last_slash_pos);
  do {
    auto find_file = log_dir + find_data.name;
    std::for_each(find_file.begin(), find_file.end(), towlower);
    all_log_file.push_back(std::move(find_file));
  } while (_wfindnext(find_handler, &find_data) == 0);
  auto now = time(nullptr);
  now -= (30 * 24 * 60 * 60); // push back 30 days
  tm past = {0};
  localtime_s(&past, &now);
  past.tm_year += 1900;
  past.tm_mon += 1;
  wchar_t base[MAX_PATH] = {0};
  swprintf_s(base, _countof(base), L"%s%04u%02u%02u.log", file_pre_path_.c_str(), past.tm_year, past.tm_mon, past.tm_mday);
  std::wstring delete_base(base);
  std::for_each(delete_base.begin(), delete_base.end(), towlower);
  for (const auto& i : all_log_file) {
    if (i < delete_base) {
      DeleteFile(i.c_str());
    }
  }
  return true;
}

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