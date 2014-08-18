/************************************************************************/
/*  Global Functions                                                    */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_UTILITY_H_
#define UTILITY_UTILITY_H_

#include <string>

namespace utility {

// Convert ASCII string to wide string, consider special format: UTF-8
std::wstring AStringToW(const std::string& a_string, bool utf8);

// Convert wide string to ASCII string
std::string WStringToA(const std::wstring& w_string);

// Get the current executable file name, without extension
std::wstring GetExeName();

// Get the current executable file folder path, without last slash
std::wstring GetExeDirectory();

// Get the current executable file full path, include extension
std::wstring GetExePath();

// Get the number of current computer processor
int GetProcessorNum();

} // namespace utility

#endif // UTILITY_UTILITY_H_