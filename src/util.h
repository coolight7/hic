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

/**
 * ## 断言
 * - 强制需要显式 == != > < 判断
 */
#define Assert_d(inbool)                                                                           \
  {                                                                                                \
    const auto str = std::string_view(#inbool);                                                    \
    assert(str.contains("=") || str.contains(">") || str.contains("<"));                           \
    assert(inbool);                                                                                \
  }

class HicUtil_c {
public:
  inline static const std::string emptyString{};
};

#define UtilPrintLine(level, path, line, tip, ...)                                                 \
  {                                                                                                \
    std::cout << std::format("[{}] {}, line({}):", HicLogLevel_c::toName(level), path, line)       \
              << std::endl;                                                                        \
    std::cout << '\t' << std::format(tip, ##__VA_ARGS__) << std::endl;                             \
  }
