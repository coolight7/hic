#pragma once

#include <array>
#include <format>
#include <iostream>
#include <memory>
#include <string>

#include "magic/macro.h"

GENERATE_ENUM(HicLogLevel, error, warning, info, debug, close)

#include "magic/unset_macro.h"

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
