#pragma once

#include <cxxabi.h>
#include <execinfo.h>

#include <string>

#if __unix__
inline std::string addr_to_symbol(void *addr) {
  if (addr == nullptr) {
    return "null";
  }
  char **strings = backtrace_symbols(&addr, 1);
  if (strings == nullptr) {
    return "???";
  }
  std::string ret = strings[0];
  free(strings);
  auto pos = ret.find('(');
  if (pos != std::string::npos) {
    auto pos2 = ret.find('+', pos);
    if (pos2 != std::string::npos) {
      auto pos3 = ret.find(')', pos2);
      auto offset = ret.substr(pos2, pos3 - pos2);
      if (pos2 != pos + 1) {
        ret = ret.substr(pos + 1, pos2 - pos - 1);
        char *demangled =
            abi::__cxa_demangle(ret.data(), nullptr, nullptr, nullptr);
        if (demangled) {
          ret = demangled;
          free(demangled);
        } else {
          ret += "()";
          ret += offset;
        }
      } else {
        ret = ret.substr(0, pos) + offset;
        auto slash = ret.rfind('/');
        if (slash != std::string::npos) {
          ret = ret.substr(slash + 1);
        }
      }
    }
  }
  return ret;
}
#else
#endif