#pragma once

#include <format>
#include <iostream>
#include <memory>

#define UtilPrintLine(level, path, line, tip, ...)                                                 \
  {                                                                                                \
    std::cout << std::format("[{}] {}, line({}):", HicUtil_c::logLevelNameList[level], path, line) \
              << std::endl;                                                                        \
    std::cout << '\t' << std::format(tip, ##__VA_ARGS__) << std::endl;                             \
  }

enum HicLogLevel_e {
  Lerror,
  Lwarning,
  Linfo,
  Ldebug,
  Lclose,
};

class HicUtil_c {
public:
  inline static const std::array<const std::string, HicLogLevel_e::Lclose + 1> logLevelNameList = {
      "error", "warning", "info", "debug", "close",
  };
};