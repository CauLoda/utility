#include "utility_net.h"
#include <vector>
#ifdef WIN32
#else
#include "utility.h"
#endif

namespace utility {

std::string ConvertIP(unsigned long ip) {
  int item[4] = {0};
  for (auto i = 0; i < 4; ++i) {
    item[i] = ((ip >> (i * 8)) & 0xFF);
  }
  char ip_str[16] = {0};
  sprintf_s(ip_str, _countof(ip_str), "%d.%d.%d.%d", item[3], item[2], item[1], item[0]);
  return ip_str;
}

unsigned long ConvertIP(const std::string& ip) {
  std::vector<int> item;
  auto left_part = ip;
  do {
    auto first_dot_pos = left_part.find('.');
    std::string each_part;
    if (first_dot_pos != std::string::npos) {
      each_part.assign(left_part, 0, first_dot_pos);
      left_part.erase(0, first_dot_pos + 1);
    } else {
      each_part = left_part;
      left_part.clear();
    }
    item.push_back(atoi(each_part.c_str()));
  } while (!left_part.empty());
  if (item.size() != 4) {
    return 0;
  }
  unsigned long int_ip = item[0] << 24 | item[1] << 16 | item[2] << 8 | item[3];
  return int_ip;
}

void ToSockAddr(const std::string& ip, int port, sockaddr_in& addr) {
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(ConvertIP(ip));
  addr.sin_port = htons(static_cast<u_short>(port));
}

void FromSockAddr(const sockaddr_in& addr, std::string& ip, int& port) {
  ip = ConvertIP(ntohl(addr.sin_addr.s_addr));
  port = ntohs(addr.sin_port);
}

} // namespace utility