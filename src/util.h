#pragma once

#include <array>
#include <cassert>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "magic/macro.h"

GENERATE_ENUM(HicLogLevel, error, warning, info, debug, close)

#include "magic/unset_macro.h"

namespace std {
inline std::string format() { return ""; }
template <typename _T> inline std::string format(_T arg) {
  std::cout << arg << std::endl;
  return "";
}
}; // namespace std

/**
 * ## 断言
 * - 强制需要显式 == != > < 判断
 */
#define Assert_d(inbool, ...)                                                                      \
  {                                                                                                \
    const auto __inbool_code = std::string_view(#inbool);                                          \
    assert(__inbool_code.contains("=") || __inbool_code.contains(">") ||                           \
           __inbool_code.contains("<"));                                                           \
    const bool rebool = (inbool);                                                                  \
    if (false == rebool) {                                                                         \
      std::cout << std::format(__VA_ARGS__) << std::endl;                                          \
      assert(rebool&& #inbool);                                                                    \
    }                                                                                              \
  }

class HicUtil_c {
public:
  inline static const std::string emptyString{};
};

#define UtilLog(level, path, line, tip, ...)                                                       \
  {                                                                                                \
    std::cout << std::format("[{}] {}, line({}):", HicLogLevel_c::toName(HicLogLevel_e::level),    \
                             path, line)                                                           \
              << std::endl;                                                                        \
    std::cout << '\t' << std::format(tip, ##__VA_ARGS__) << std::endl;                             \
  }

enum ListNodeType_e {
  Lexical,
  Syntactic,
};

class ListNode_c {
public:
  ListNode_c(ListNodeType_e in_type) : nodeType(in_type) {}
  ListNode_c(const ListNode_c&) = delete;

  virtual void printInfo() const {}
  virtual const std::string& name() const { return HicUtil_c::emptyString; }

  ListNodeType_e nodeType;
};
