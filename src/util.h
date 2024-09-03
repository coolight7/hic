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

void assertPrint() {}
template <typename _T> constexpr void assertPrint(_T arg) { std::cout << arg << std::endl; }
template <typename... _Args> constexpr void assertPrint(const char* str, _Args&&... args) {
  std::cout << std::format(str, std::forward<_Args>(args)...) << std::endl;
}
/**
 * ## 断言
 * - 强制需要显式 == != > < 判断
 */
#define Assert_d(inbool, ...)                                                                      \
  {                                                                                                \
    const auto str = std::string_view(#inbool);                                                    \
    assert(str.contains("=") || str.contains(">") || str.contains("<"));                           \
    const auto rebool = inbool;                                                                    \
    if (false == rebool) {                                                                         \
      assertPrint(__VA_ARGS__);                                                                    \
    }                                                                                              \
    assert(inbool);                                                                                \
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

  ListNodeType_e nodeType;
};