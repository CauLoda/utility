/************************************************************************/
/*  Log Interface                                                       */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_LOG_H_
#define UTILITY_LOG_H_

// Distinguish between different types of logs
enum LogLevel {
  kStartup = 1,
  kShutdown = 2,
  kInfo = 4,
  kWarning = 8,
  kError = 16
};

void logging(LogLevel level,
  const char* file_name,
  int line_number,
  const char* function_name,
  const char* format_str,
  ...);

void InitLog(const char* init_info, int log_level = kStartup | kShutdown | kInfo | kWarning | kError);

// The macro for logging
// USAGE: LOG(kInfo, "Hello, %s!", "World");
#define LOG(level, ...) logging(level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#endif // UTILITY_LOG_H_