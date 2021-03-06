/************************************************************************/
/*  Net Global Functions                                                */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_UTILITY_NET_H_
#define UTILITY_UTILITY_NET_H_

#include <string>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

namespace utility {

// Convert byte format IP to string format
std::string ConvertIP(unsigned long ip);

// Convert string format IP to byte format
unsigned long ConvertIP(const std::string& ip);

// Convert socket address
void ToSockAddr(const std::string& ip, int port, sockaddr_in& addr);

// Convert socket address
void FromSockAddr(const sockaddr_in& addr, std::string& ip, int& port);

} // namespace utility

#endif // UTILITY_UTILITY_NET_H_