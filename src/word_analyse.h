#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "util.h"

#define WordPrintLine(level, tip, ...) UtilPrintLine(level, "", current_line, tip, __VA_ARGS__)

enum WordValueToken_e {
  Tundefined,
  Tnumber,
  Tstring,
  Tsign,
  Tkeyword,
  Ttype,
  Tfunction,
  TnativeCall,
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
  WordItem_c() : token(WordValueToken_e::Tundefined) {}

  WordItem_c(WordValueToken_e in_token, const std::string& in_name)
      : token(in_token), name(in_name) {}

  WordValueToken_e token;
  std::string name;
};

class WordAnalyse_c {
public:
  inline static const std::string emptyString{};
  inline static const std::array<const std::string, 12> reserveKeywords_ctrl = {
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
  // 类型
  inline static const std::array<const std::string, 8> reserveKeywords_type = {
      "void", "char", "bool", "int", "int64", "float", "float64", "class",
  };
  // 内置函数
  inline static const std::array<const std::string, 6> reserveKeywords_nativeCall = {
      "print", "sizeof", "malloc", "free", "exit", "main",
  };

  template <size_t _NUM>
  inline static const std::string&
  isReserveKeyWord(std::string_view it_ptr,
                   const std::array<const std::string, _NUM> reserveKeywords) {
    for (const auto& item : reserveKeywords) {
      if (it_ptr.starts_with(item)) {
        return item;
      }
    }
    return emptyString;
  }

  // 判断是否是关键字(控制)
  inline static const std::string& isReserveKeyWord_ctrl(std::string_view it_ptr) {
    return isReserveKeyWord(it_ptr, reserveKeywords_ctrl);
  }

  // 判断是否是关键字(类型)
  inline static const std::string& isReserveKeyWord_type(std::string_view it_ptr) {
    return isReserveKeyWord(it_ptr, reserveKeywords_type);
  }

  // 判断是否是关键字(内置函数)
  inline static const std::string& isReserveKeyWord_nativeCall(std::string_view it_ptr) {
    return isReserveKeyWord(it_ptr, reserveKeywords_nativeCall);
  }

  std::map<std::string, WordItem_c>& currentSymbolTable() {
    assert(symbolTable.empty() == false);
    return symbolTable.back();
  }

  void symbolTablePush() { symbolTable.emplace_back(std::map<std::string, WordItem_c>{}); }
  void symbolTablePop() { symbolTable.pop_back(); }
  // 在 [symbolTable] 中查找符号，且是从最近/最小的作用域开始查找
  std::optional<WordItem_c> symbolTableFind(const std::string& key) {
    if (false == symbolTable.empty()) {
      for (auto it = symbolTable.end() - 1; it != symbolTable.begin(); --it) {
        if (it->contains(key)) {
          return (*it)[key];
        }
      }
    }
    return std::nullopt;
  }

  std::optional<WordItem_c> currentSymbolTableFind(const std::string& key) {
    auto& curr = currentSymbolTable();
    if (curr.contains(key)) {
      return curr[key];
    }
  }

  void init(std::string_view in_code) {
    raw_code = in_code;
    code_it = raw_code.begin();
    current_line = 1;
    symbolTable.clear();
    symbolTablePush();
    auto& currTable = currentSymbolTable();
    // 添加关键字
    for (const auto& item : reserveKeywords_ctrl) {
      currTable[item] = WordItem_c{WordValueToken_e::Tkeyword, item};
    }
    // 添加类型
    for (const auto& item : reserveKeywords_type) {
      currTable[item] = WordItem_c{WordValueToken_e::Ttype, item};
    }
    // 添加内置函数
    for (int i = 0; i < reserveKeywords_nativeCall.size(); ++i) {
      const auto& item = reserveKeywords_nativeCall[i];
      auto word = WordItem_c{WordValueToken_e::TnativeCall, item};
      currTable[item] = word;
    }
  }

  // <isSuccess, str>
  std::optional<WordItem_c> analyse() {
    for (; code_it != raw_code.end();) {
      const auto it = *code_it;
      ++code_it;
      if (' ' == it || '\t' == it || '\r' == it) {
        continue;
      }
      if ('\n' == it) {
        ++current_line;
        continue;
      }
      {
        // 名称
        bool isLowcase = (it >= 'a' && it <= 'z');
        if (isLowcase || (it >= 'A' && it <= 'Z') || '_' == it) {
          const char* start = code_it - 1;
          const char* end = nullptr;
          while (code_it != raw_code.end() &&
                 ((*code_it >= 'a' && *code_it <= 'z') || (*code_it >= 'A' && *code_it <= 'Z') ||
                  '_' == *code_it)) {
            ++code_it;
          }
          end = code_it;
          const auto name_view = std::string_view{start, end};
          WordValueToken_e token = WordValueToken_e::Tid;
          auto name = std::string{name_view};
          // 查找是否已经存在该符号
          auto result = symbolTableFind(name);
          if (result.has_value()) {
            return result.value();
          } else {
            // 添加符号
            auto item = WordItem_c{WordValueToken_e::Tid, name};
            currentSymbolTable()[name] = item;
            return item;
          }
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
                return std::nullopt;
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
        // 字符串
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
          return std::nullopt;
        } else {
          auto item = WordItem_c{WordValueToken_e::Tstring, value};
          return item;
        }
      }
      {
        // 符号
        if ('#' == it || ('/' == it && '/' == *code_it)) {
          // 宏，不处理，作为注释，换行到下一行
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
        if ('/' == it && '*' == *code_it) {
          // 多行注释
          if (raw_code.end() > code_it) {
            ++code_it;
          }
          bool hasEnd = false;
          while (raw_code.end() > code_it) {
            ++code_it;
            if ('*' == *code_it && '/' == *(code_it + 1)) {
              hasEnd = true;
              break;
            }
          }
          if (hasEnd) {
            ++code_it;
            continue;
          } else {
            WordPrintLine(HicLogLevel_e::Lerror, "多行注释 /* 缺少右边界 {}", "");
            return std::nullopt;
          }
        }
        if ('{' == it) {
          symbolTablePush();
          return WordItem_c{WordValueToken_e::Tsign, std::to_string(it)};
        }
        if ('}' == it) {
          symbolTablePop();
          return WordItem_c{WordValueToken_e::Tsign, std::to_string(it)};
        }
        if ('(' == it || ')' == it) {
          return WordItem_c{WordValueToken_e::Tsign, std::to_string(it)};
        }
        const char* start = code_it - 1;
        const char* end = code_it;
        do {
          if ('=' == it) {
            if ('=' == *code_it) {
              // ==
              start = code_it;
              ++code_it;
            }
            break;
          }
          if ('>' == it) {
            if ('=' == *code_it) {
              // >=
              start = code_it;
              ++code_it;
            }
            break;
          }
          if ('<' == it) {
            if ('=' == *code_it) {
              // <=
              start = code_it;
              ++code_it;
            }
            break;
          }
          if ('*' == it) {
            if ('=' == *code_it || '*' == *code_it) {
              // *=  **
              start = code_it;
              ++code_it;
            }
            break;
          }
          if ('/' == it) {
            if ('=' == *code_it) {
              // /=
              start = code_it;
              ++code_it;
            }
            break;
          }
        } while (false);
        assert(nullptr != end);
        return WordItem_c{WordValueToken_e::Tsign, std::string{start, end}};
      }
    }
    return std::nullopt;
  }

  void debugPrint() {
    int i = 0;
    for (const auto& table : symbolTable) {
      std::cout << i + 1 << std::endl;
      for (const auto& it : table) {
        for (auto j = i; j-- > 0;) {
          std::cout << "  ";
        }
        std::cout << "- ";
        std::cout << it.first << ":    \t" << it.second.name << "\t" << it.second.token
                  << std::endl;
      }
    }
  }

  int current_line = 1;
  std::string_view raw_code;
  const char* code_it = nullptr;
  // 作用域符号表
  std::vector<std::map<std::string, WordItem_c>> symbolTable{};
};