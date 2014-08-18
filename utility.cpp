#include "utility.h"
#include <memory>
#include <Windows.h>

namespace utility {

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

std::string WStringToA(const std::wstring& w_string) {
  if (w_string.empty()) {
    return "";
  }
  auto string_size = WideCharToMultiByte(CP_OEMCP, 0, w_string.c_str(), -1, NULL, 0, NULL, NULL);
  std::unique_ptr<char[]> a_string(new char[string_size]);
  WideCharToMultiByte(CP_OEMCP, 0, w_string.c_str(), -1, a_string.get(), string_size, NULL, NULL);
  return a_string.get();
}

std::wstring GetExeName() {
  auto exe_path = GetExePath();
  auto last_slash_pos = exe_path.rfind(L'\\');
  auto dot_exe_pos = exe_path.rfind(L".exe");
  auto exe_name_length = dot_exe_pos - ++last_slash_pos;
  std::wstring exe_name(exe_path, last_slash_pos, exe_name_length);
  return std::move(exe_name);
}

std::wstring GetExePath() {
  wchar_t exe_path[MAX_PATH] = {0};
  GetModuleFileName(NULL, exe_path, _countof(exe_path));
  return exe_path;
}

std::wstring GetExeDirectory() {
  auto exe_path = GetExePath();
  auto last_slash_pos = exe_path.rfind(L'\\');
  std::wstring exe_dir(exe_path, 0, last_slash_pos);
  return std::move(exe_dir);
}

int GetProcessorNum() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}

} // namespace utility