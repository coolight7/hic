#pragma once

#include <array>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "util.h"

#define WordPrintLine(level, tip, ...) UtilPrintLine(level, "", current_line, tip, __VA_ARGS__)

enum WordValueToken_e {
  TUndefined,
  Tnumber,
  TString,
  TKeyword,
  Tfunction,
  Tsyscall,
  Tid,
};

/**
 * 变量类型
 */
enum WordValueType_e {
  Tvoid,
  Tchar,
  Tbool,
  Tint,
  Tint64,
  Tfloat,
  Tfloat64,
  Tpointer,
  Tclass,
};

class WordValue_c {
public:
  WordValue_c(WordValueType_e in_type, int64_t in_value) : value(in_value), valueType(in_type) {}

  int64_t value = 0;
  WordValueType_e valueType;
};

class WordItem_c {
public:
  WordItem_c() : token(WordValueToken_e::TUndefined) {}

  WordItem_c(WordValueToken_e in_token, const std::string& in_name)
      : token(in_token), name(in_name) {}

  WordValueToken_e token;
  std::string name;
  // 作用域变量声明
  std::vector<WordValue_c> valuelist{};
};

class WordAnalyse_c {
public:
  inline static const std::string emptyString{};
  inline static const std::array<std::string, 20> reserveKeywords = {
      // 类型
      "void",
      "char",
      "bool",
      "int",
      "int64",
      "float",
      "float64",
      "class",
      // 范围
      "const",
      "static",
      // 控制
      "if",
      "else",
      "switch",
      "case",
      "default",
      "break",
      "while",
      "do",
      "for",
      "return",
  };

  inline static const std::array<std::string, 6> reserveKeywords_nativeCall = {
      "print", "sizeof", "malloc", "free", "exit", "main",
  };

  /**
   * 判断是否是关键字
   */
  inline static const std::string& isReserveKeyWord(std::string_view it_ptr) {
    for (const auto& item : reserveKeywords) {
      if (it_ptr.starts_with(item)) {
        return item;
      }
    }
    return emptyString;
  }

  void init(std::string_view in_code) {
    raw_code = in_code;
    code_it = raw_code.begin();
    current_line = 1;
    symbolTable.clear();
    // 添加关键字符号
    for (const auto& item : reserveKeywords) {
      symbolTable[item] = WordItem_c{WordValueToken_e::TKeyword, item};
    }
    // 添加内置函数
    for (int i = 0; i < reserveKeywords_nativeCall.size(); ++i) {
      const auto& item = reserveKeywords_nativeCall[i];
      auto word = WordItem_c{WordValueToken_e::Tsyscall, item};
      word.valuelist.emplace_back(WordValue_c{
          WordValueType_e::Tint,
          i,
      });
      symbolTable[item] = word;
    }
  }

  // <isSuccess, str>
  WordItem_c analyse() {
    for (; code_it != raw_code.end();) {
      const auto it = *code_it;
      ++code_it;
      if ('\n' == it) {
        ++current_line;
        continue;
      }
      if (' ' == it || '\t' == it || '\r' == it) {
        continue;
      }
      if ('#' == it) {
        // 换行到下一行
        while (raw_code.end() != code_it) {
          ++code_it;
          if ('\n' == *code_it) {
            ++code_it;
            break;
          }
        }
        ++current_line;
        continue;
      }
      {
        // 符号
        const bool isLowaz = (it >= 'a' && it <= 'z');
        if (isLowaz || (it >= 'A' && it <= 'Z') || '_' == it) {
        }
      }
      if (it >= '0' && it <= '9') {
        // 数值
        long long value = it - '0';
        int step = 10;
        if ('0' == it) {
          if ('x' == *code_it) {
            // 16进制 0x
            step = 16;
            ++code_it;
          } else if ('1' <= *code_it && '9' >= *code_it) {
            // 8进制 077
            step = 8;
          } else if ('0' == *code_it) {
            // 连续 0，警告！
            WordPrintLine(HicLogLevel_e::Lwarning,
                          "数值开头不应使用连续的0；该值将被认为是十进制{}", "");
          }
        }
        do {
          int item = 0;
          if (step == 16) {
            // 十六进制
            if ('a' <= *code_it && 'f' >= *code_it) {
              item = *code_it - 'a' + 10;
            } else if ('A' <= *code_it && 'F' >= *code_it) {
              item = *code_it - 'A' + 10;
            } else if ('0' <= *code_it && '9' >= *code_it) {
              item = *code_it - '0';
            } else {
              break;
            }
          } else if (step == 8) {
            // 八进制
            if ('0' <= *code_it && '7' >= *code_it) {
              item = *code_it - '0';
            } else {
              if ('8' == *code_it || '9' == *code_it) {
                WordPrintLine(HicLogLevel_e::Lerror, "预期为8进制的数值，却包含了字符：{}",
                              *code_it);
                return WordItem_c{};
              }
              break;
            }
          } else {
            if ('0' <= *code_it && '9' >= *code_it) {
              item = *code_it - '0';
            } else {
              break;
            }
          }
          value = value * step + item;
          ++code_it;
        } while (raw_code.end() != code_it);
        auto item = WordItem_c{WordValueToken_e::Tnumber, std::to_string(value)};
        return item;
      }
      if ('"' == it || '\'' == it) {
        std::string value;
        const char startSign = it;
        const char* start = code_it;
        const char* end = nullptr;
        bool isShift = false; // 前一个字符是否是转义符号
        for (; code_it != raw_code.end(); ++code_it) {
          if (isShift) {
            // 有前置转义
            switch (*code_it) {
            case 'r':
            case '\r':
              value += '\r';
              break;
            case 'n':
            case '\n':
              value += '\n';
              break;
            case 't':
              value += '\t';
              break;
            default:
              value += *code_it;
              break;
            }
            isShift = false;
            continue;
          } else {
            // 无前置转义
            if (startSign == *code_it) {
              end = code_it;
              ++code_it;
              break;
            } else if ('\n' == *code_it) {
              // 没有前置转义且遇到换行
              break;
            }
          }
          if ('\\' == *code_it) {
            // 非连续转义，且当前为转义
            isShift = true;
          } else {
            value += *code_it;
          }
        }
        if (nullptr == end) {
          WordPrintLine(HicLogLevel_e::Lerror, "字符串缺少右边界：{}", startSign);
          return WordItem_c{};
        } else {
          auto item = WordItem_c{WordValueToken_e::TString, value};
          return item;
        }
      }
    }
    return WordItem_c{};
  }

  void debugPrint() {
    for (const auto& it : symbolTable) {
      std::cout << it.first << ":    \t" << it.second.name << "\t" << it.second.token << std::endl;
    }
  }

  int current_line = 1;
  std::string_view raw_code;
  const char* code_it = nullptr;
  std::map<std::string, WordItem_c> symbolTable{};
};