#pragma once

#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "rule.h"
#include "util.h"

#define WordPrintLine(level, tip, ...) UtilPrintLine(level, "", current_line, tip, __VA_ARGS__)

class WordValue_c {
public:
  WordValue_c(WordEnumType_e in_type, int64_t in_value) : value(in_value), valueType(in_type) {}

  int64_t value = 0;
  WordEnumType_e valueType;
};

class WordItem_default_c;
class WordItem_number_c;
class WordItem_ctrl_c;
class WordItem_type_c;
class WordItem_nativeCall_c;

class WordItem_c {
public:
  WordItem_c() : token(WordEnumToken_e::Tundefined) {}

  WordItem_c(WordEnumToken_e in_token) : token(in_token) {}
  WordItem_c(const WordItem_c&) = delete;

  virtual const std::string& name() const { return HicUtil_c::emptyString; }

  template <typename _T, typename... _Args>
  static std::shared_ptr<WordItem_c> make_shared(_Args... args) {
    _T* ptr = new _T{std::forward<_Args>(args)...};
    return std::shared_ptr<WordItem_c>(ptr);
  }

  WordItem_default_c& toDefault() {
    Assert_d(WordEnumToken_e::Tstring == token || WordEnumToken_e::Tsign == token ||
             WordEnumToken_e::Tid == token);
    return *((WordItem_default_c*)this);
  }
  WordItem_number_c& toNumber() {
    Assert_d(WordEnumToken_e::Tnumber == token);
    return *((WordItem_number_c*)this);
  }
  WordItem_ctrl_c& toKeyword() {
    Assert_d(WordEnumToken_e::Tkeyword == token);
    return *((WordItem_ctrl_c*)this);
  }
  WordItem_type_c& toType() {
    Assert_d(WordEnumToken_e::Ttype == token);
    return *((WordItem_type_c*)this);
  }
  WordItem_nativeCall_c& toNativeCall() {
    Assert_d(WordEnumToken_e::TnativeCall == token);
    return *((WordItem_nativeCall_c*)this);
  }

  virtual ~WordItem_c() {}

  WordEnumToken_e token;
};

// string
// sign
// id
class WordItem_default_c : public WordItem_c {
public:
  WordItem_default_c(WordEnumToken_e in_type, const std::string& in_value)
      : WordItem_c(in_type), value(in_value) {}

  const std::string& name() const override { return value; }

  std::string value;
};

class WordItem_number_c : public WordItem_c {
public:
  WordItem_number_c(long long in_value) : WordItem_c(WordEnumToken_e::Tnumber), value(in_value) {
    name_ = std::to_string(in_value);
  }

  const std::string& name() const override { return name_; }

  std::string name_;
  long long value;
};

class WordItem_ctrl_c : public WordItem_c {
public:
  WordItem_ctrl_c(WordEnumCtrl_e in_value)
      : WordItem_c(WordEnumToken_e::Tkeyword), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumCtrl_c::namelist.size());
    return WordEnumCtrl_c::namelist[value];
  }

  // index
  WordEnumCtrl_e value;
};

class WordItem_type_c : public WordItem_c {
public:
  WordItem_type_c(WordEnumType_e in_value) : WordItem_c(WordEnumToken_e::Ttype), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumType_c::namelist.size());
    return WordEnumType_c::namelist[value];
  }

  WordEnumType_e value;
};

class WordItem_nativeCall_c : public WordItem_c {
public:
  WordItem_nativeCall_c(WordEnumNativeCall_e in_value)
      : WordItem_c(WordEnumToken_e::TnativeCall), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumNativeCall_c::namelist.size());
    return WordEnumNativeCall_c::namelist[value];
  }

  // index
  WordEnumNativeCall_e value;
};

class LexicalAnalyse_c {
public:
  template <size_t _NUM>
  inline static const std::string&
  isReserveKeyWord(std::string_view it_ptr,
                   const std::array<const std::string, _NUM> reserveKeywords) {
    for (const auto& item : reserveKeywords) {
      if (it_ptr.starts_with(item)) {
        return item;
      }
    }
    return HicUtil_c::emptyString;
  }

  // 判断是否是关键字(控制)
  inline static const std::string& isReserveKeyWord_ctrl(std::string_view it_ptr) {
    return isReserveKeyWord(it_ptr, WordEnumCtrl_c::namelist);
  }

  // 判断是否是关键字(类型)
  inline static const std::string& isReserveKeyWord_type(std::string_view it_ptr) {
    return isReserveKeyWord(it_ptr, WordEnumType_c::namelist);
  }

  // 判断是否是关键字(内置函数)
  inline static const std::string& isReserveKeyWord_nativeCall(std::string_view it_ptr) {
    return isReserveKeyWord(it_ptr, WordEnumNativeCall_c::namelist);
  }

  std::map<const std::string, std::shared_ptr<WordItem_c>>& currentSymbolTable() {
    Assert_d(symbolTable.empty() == false);
    return symbolTable.back();
  }

  void symbolTablePush() {
    symbolTable.emplace_back(std::map<const std::string, std::shared_ptr<WordItem_c>>{});
  }

  void symbolTablePop() { symbolTable.pop_back(); }
  // 在 [symbolTable] 中查找符号，且是从最近/最小的作用域开始查找
  std::shared_ptr<WordItem_c> symbolTableFind(const std::string& key) {
    if (false == symbolTable.empty()) {
      for (auto it = symbolTable.end() - 1;; --it) {
        if (it->contains(key)) {
          return (*it)[key];
        }
        if (it == symbolTable.begin()) {
          break;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> currentSymbolTableFind(const std::string& key) {
    auto& curr = currentSymbolTable();
    if (curr.contains(key)) {
      return curr[key];
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> currentToken() {
    Assert_d(false == tokenList.empty());
    return tokenList.back();
  }

  std::shared_ptr<WordItem_c> getToken(int index) {
    Assert_d(index >= 0 && index < tokenList.size());
    return tokenList[index];
  }

  void init(std::string_view in_code) {
    raw_code = in_code;
    code_it = raw_code.begin();
    current_line = 1;
    symbolTable.clear();
    symbolTablePush();
    auto& currTable = currentSymbolTable();
    // 添加关键字
    for (int i = 0; i < WordEnumCtrl_c::namelist.size(); ++i) {
      const auto& item = WordEnumCtrl_c::namelist[i];
      currTable[item] = WordItem_c::make_shared<WordItem_ctrl_c>(WordEnumCtrl_c::toEnum(i));
    }
    // 添加类型
    for (int i = 0; i < WordEnumType_c::namelist.size(); ++i) {
      const auto& item = WordEnumType_c::namelist[i];
      currTable[item] = WordItem_c::make_shared<WordItem_type_c>(WordEnumType_c::toEnum(i));
    }
    // 添加内置函数
    for (int i = 0; i < WordEnumNativeCall_c::namelist.size(); ++i) {
      const auto& item = WordEnumNativeCall_c::namelist[i];
      currTable[item] =
          WordItem_c::make_shared<WordItem_nativeCall_c>(WordEnumNativeCall_c::toEnum(i));
    }
  }

  std::shared_ptr<WordItem_c> _innerAnalyse() {
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
          WordEnumToken_e token = WordEnumToken_e::Tid;
          auto name = std::string{name_view};
          // 查找是否已经存在该符号
          auto result = symbolTableFind(name);
          if (nullptr != result) {
            return result;
          } else {
            // 添加符号
            auto item = WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tid, name);
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
            WordPrintLine(HicLogLevel_e::Twarning,
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
                WordPrintLine(HicLogLevel_e::Terror, "预期为8进制的数值，却包含了字符：{}",
                              *code_it);
                return nullptr;
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
        return WordItem_c::make_shared<WordItem_number_c>(value);
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
          WordPrintLine(HicLogLevel_e::Terror, "字符串缺少右边界：{}", startSign);
          return nullptr;
        } else {
          return WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tstring, value);
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
          bool hasEnd = false;
          while (raw_code.end() > code_it) {
            ++code_it;
            if ('\n' == *code_it) {
              ++current_line;
            } else if ('*' == *code_it && '/' == *(code_it + 1)) {
              hasEnd = true;
              code_it += 2;
              break;
            }
          }
          if (hasEnd) {
            continue;
          } else {
            WordPrintLine(HicLogLevel_e::Terror, "多行注释 /* 缺少右边界 {}", "");
            return nullptr;
          }
        }
        if ('{' == it) {
          symbolTablePush();
          return WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tsign,
                                                             std::string(&it, 1));
        }
        if ('}' == it) {
          symbolTablePop();
          return WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tsign,
                                                             std::string(&it, 1));
        }
        if ('(' == it || ')' == it) {
          return WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tsign,
                                                             std::string(&it, 1));
        }
        const char* start = code_it - 1;
        const char* end = code_it;
        do {
          if ('|' == it || '&' == it) {
            if (it == *code_it || '=' == *code_it) {
              // || |=
              // && &=
              ++code_it;
              end = code_it;
              ++code_it;
            }
            break;
          }
          if ('=' == it || '>' == it || '<' == it || '+' == it || '-' == it || '*' == it ||
              '/' == it) {
            if ('=' == *code_it) {
              // == >= += -= *= /=
              ++code_it;
              end = code_it;
              ++code_it;
            }
            break;
          }
          if ('?' == it) {
            if ('?' == *code_it) {
              // ??
              ++code_it;
              if ('=' == *code_it) {
                // ??=
                ++code_it;
              }
            } else if ('.' == *code_it) {
              // ?.
              ++code_it;
            }
            end = code_it;
          }
        } while (false);
        Assert_d(nullptr != end);
        return WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tsign,
                                                           std::string{start, end});
      }
    }
    return nullptr;
  }

  // <isSuccess, str>
  std::shared_ptr<WordItem_c> analyse() {
    auto result = _innerAnalyse();
    if (nullptr != result) {
      tokenList.push_back(result);
    }
    return result;
  }

  void debugPrintSymbolTable(bool printKeyword) {
    int i = 0;
    for (const auto& table : symbolTable) {
      std::cout << i + 1 << std::endl;
      for (const auto& it : table) {
        if (false == printKeyword && (it.second->token == WordEnumToken_e::Tkeyword ||
                                      it.second->token == WordEnumToken_e::Ttype ||
                                      it.second->token == WordEnumToken_e::TnativeCall)) {
          continue;
        }
        for (auto j = i; j-- > 0;) {
          std::cout << "  ";
        }
        std::cout << "- ";
        std::cout << it.first << ":    \t" << it.second->name() << "\t"
                  << WordEnumToken_c::toName(it.second->token) << std::endl;
      }
    }
  }

  static void debugPrintSymbol(const WordItem_c& word) {
    std::cout << WordEnumToken_c::toName(word.token) << "\t" << word.name() << std::endl;
  }

  void debugPrintSymbolList() {
    for (const auto& item : tokenList) {
      const auto& word = *item.get();
      debugPrintSymbol(word);
    }
  }

  int current_line = 1;
  std::string_view raw_code;
  const char* code_it = nullptr;
  // 作用域符号表
  std::vector<std::map<const std::string, std::shared_ptr<WordItem_c>>> symbolTable{};
  std::vector<std::shared_ptr<WordItem_c>> tokenList{};
};