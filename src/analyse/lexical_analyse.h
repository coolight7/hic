#pragma once

// 词法分析

#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "rule.h"
#include "util.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 `,逗号` 给 UtilLog导致展开异常
 */
#define WordLog(level, tip, ...) UtilLog(level, "", current_line, tip, ##__VA_ARGS__)

class WordValue_c {
public:
  WordValue_c(WordEnumType_e in_type, int64_t in_value) : value(in_value), valueType(in_type) {}

  int64_t value = 0;
  WordEnumType_e valueType;
};

class WordItem_default_c;
class WordItem_operator_c;
class WordItem_number_c;
class WordItem_ctrl_c;
class WordItem_type_c;
class WordItem_nativeCall_c;

class WordItem_c : public ListNode_c {
public:
  WordItem_c() : ListNode_c(ListNodeType_e::Lexical), token(WordEnumToken_e::Tundefined) {}

  WordItem_c(WordEnumToken_e in_token) : ListNode_c(ListNodeType_e::Lexical), token(in_token) {}
  WordItem_c(const WordItem_c&) = delete;

  const std::string& name() const override { return HicUtil_c::emptyString; }

  void printInfo() const override {
    const auto& str = WordEnumToken_c::toName(token);
    std::cout << str << " ";
    int fill = 10 - str.size();
    if (fill > 0) {
      while (fill-- > 0) {
        std::cout << "-";
      }
    }
    std::cout << " " << name() << std::endl;
  }

  template <typename _T, typename... _Args>
  static std::shared_ptr<WordItem_c> make_shared(_Args... args) {
    _T* ptr = new _T{std::forward<_Args>(args)...};
    return std::shared_ptr<WordItem_c>(ptr);
  }

  WordItem_default_c& toDefault() const {
    Assert_d(WordEnumToken_e::Tstring == token || WordEnumToken_e::Tid == token);
    return *((WordItem_default_c*)this);
  }
  WordItem_operator_c& toOperator() const {
    Assert_d(WordEnumToken_e::Toperator == token);
    return *((WordItem_operator_c*)this);
  }
  WordItem_number_c& toNumber() const {
    Assert_d(WordEnumToken_e::Tnumber == token);
    return *((WordItem_number_c*)this);
  }
  WordItem_ctrl_c& toKeyword() const {
    Assert_d(WordEnumToken_e::Tkeyword == token);
    return *((WordItem_ctrl_c*)this);
  }
  WordItem_type_c& toType() const {
    Assert_d(WordEnumToken_e::Ttype == token);
    return *((WordItem_type_c*)this);
  }
  WordItem_nativeCall_c& toNativeCall() const {
    Assert_d(WordEnumToken_e::TnativeCall == token);
    return *((WordItem_nativeCall_c*)this);
  }

  virtual ~WordItem_c() {}

  std::string toString() const {
    return std::format("[{}] {}", WordEnumToken_c::toName(token), name());
  }

  WordEnumToken_e token;
};

// string
// sign
// id
class WordItem_default_c : public WordItem_c {
public:
  WordItem_default_c(WordEnumToken_e in_type, const std::string_view& in_value)
      : WordItem_c(in_type), value(in_value) {}

  const std::string& name() const override { return value; }

  std::string value;
};

class WordItem_operator_c : public WordItem_c {
public:
  WordItem_operator_c(WordEnumOperator_e in_value)
      : WordItem_c(WordEnumToken_e::Toperator), value(in_value) {}

  const std::string& name() const override { return toSign(value); }

  // `R"()"` fix warning `trigraph ??= ignored, use -trigraphs to enable`
  inline static const std::array<const std::string, 47> signlist = {
      "[Undefine]", "",       "expr++", "expr--", "(",      ")",      "[",      "]",   ".",
      "?.",         "Level1", "!expr",  "~expr",  "++expr", "--expr", "Level2", "*",   "/",
      "%",          "+",      "-",      "<<",     ">>",     "&",      "|",      ">=",  ">",
      "<=",         "<",      "==",     "!=",     "&&",     "||",     "??",     "? :", "=",
      "*=",         "/=",     "+=",     "-=",     R"(??=)", "{",      "}",      ";",   ",",
  };

  inline static constexpr const std::string& toSign(WordEnumOperator_e value) {
    static_assert(WordEnumOperator_c::namelist.size() == signlist.size());
    return signlist[WordEnumOperator_c::toInt(value)];
  }

  inline static constexpr WordEnumOperator_e toEnum(const std::string_view str) {
    if ("++" == str) {
      return WordEnumOperator_e::TEndAddAdd;
    }
    if ("--" == str) {
      return WordEnumOperator_e::TEndSubSub;
    }
    if ("(" == str) {
      return WordEnumOperator_e::TLeftCurvesGroup;
    }
    if (")" == str) {
      return WordEnumOperator_e::TRightCurvesGroup;
    }
    if ("[" == str) {
      return WordEnumOperator_e::TLeftSquareGroup;
    }
    if ("]" == str) {
      return WordEnumOperator_e::TRightSquareGroup;
    }
    if ("." == str) {
      return WordEnumOperator_e::TDot;
    }
    if ("?." == str) {
      return WordEnumOperator_e::TNullDot;
    }
    if ("!" == str) {
      return WordEnumOperator_e::TNot;
    }
    if ("~" == str) {
      return WordEnumOperator_e::TShift;
    }
    if ("++" == str) {
      return WordEnumOperator_e::TStartAddAdd;
    }
    if ("--" == str) {
      return WordEnumOperator_e::TStartSubSub;
    }
    if ("*" == str) {
      return WordEnumOperator_e::TMulti;
    }
    if ("/" == str) {
      return WordEnumOperator_e::TDivision;
    }
    if ("%" == str) {
      return WordEnumOperator_e::TPercent;
    }
    if ("+" == str) {
      return WordEnumOperator_e::TAdd;
    }
    if ("-" == str) {
      return WordEnumOperator_e::TSub;
    }
    if ("<<" == str) {
      return WordEnumOperator_e::TBitLeftMove;
    }
    if (">>" == str) {
      return WordEnumOperator_e::TBitRightMove;
    }
    if ("&" == str) {
      return WordEnumOperator_e::TBitAnd;
    }
    if ("|" == str) {
      return WordEnumOperator_e::TBitOr;
    }
    if (">=" == str) {
      return WordEnumOperator_e::TGreaterOrEqual;
    }
    if (">" == str) {
      return WordEnumOperator_e::TGreater;
    }
    if ("<=" == str) {
      return WordEnumOperator_e::TLessOrEqual;
    }
    if ("<" == str) {
      return WordEnumOperator_e::TLess;
    }
    if ("==" == str) {
      return WordEnumOperator_e::TEqual;
    }
    if ("!=" == str) {
      return WordEnumOperator_e::TNotEqual;
    }
    if ("&&" == str) {
      return WordEnumOperator_e::TAnd;
    }
    if ("||" == str) {
      return WordEnumOperator_e::TOr;
    }
    if ("??" == str) {
      return WordEnumOperator_e::TNullMerge;
    }
    if ("? :" == str) {
      return WordEnumOperator_e::TIfElse;
    }
    if ("=" == str) {
      return WordEnumOperator_e::TSet;
    }
    if ("*=" == str) {
      return WordEnumOperator_e::TSetMulti;
    }
    if ("/=" == str) {
      return WordEnumOperator_e::TSetDivision;
    }
    if ("+=" == str) {
      return WordEnumOperator_e::TSetAdd;
    }
    if ("-=" == str) {
      return WordEnumOperator_e::TSetSub;
    }
    if (str == R"(??=)") {
      // `R"()"` fix warning `trigraph ??= ignored, use -trigraphs to enable`
      return WordEnumOperator_e::TSetNullMerge;
    }
    if ("{" == str) {
      return WordEnumOperator_e::TLeftFlowerGroup;
    }
    if ("}" == str) {
      return WordEnumOperator_e::TRightFlowerGroup;
    }
    if (";" == str) {
      return WordEnumOperator_e::TSemicolon;
    }
    if ("," == str) {
      return WordEnumOperator_e::TComma;
    }
    Assert_d(true == false, "未知操作符: {}", str);
    return WordEnumOperator_e::TUndefined;
  }

  inline static int toLevel(WordEnumOperator_e type) {
    Assert_d(type != WordEnumOperator_e::TUndefined && type != WordEnumOperator_e::TLevel1 &&
             type != WordEnumOperator_e::TLevel2);
    int index = WordEnumOperator_c::toInt(type);
    if (index < WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel1)) {
      return 1;
    }
    if (index < WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel2)) {
      return 2;
    }
    return index;
  }

  /**
   * ## [left] 优先级是否大于 [right]
   * - result > 0  : [left] 的优先级小于 [right]
   * - result == 0 : [left] 的优先级等于 [right]，注意此时 [left] 和 [right] 不一定是相同
   * 符号，部分符号的优先级是相等的。
   * - result < 0  : [left] 的优先级大于 [right]
   */
  inline static int compare(WordEnumOperator_e left, WordEnumOperator_e right) {
    return toLevel(left) - toLevel(right);
  }

  WordEnumOperator_e value;
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

  void init(std::string_view in_code) {
    raw_code = in_code;
    code_it = raw_code.begin();
    current_line = 1;
    reserveKeywords_.clear();
    // 添加关键字
    for (int i = 0; i < WordEnumCtrl_c::namelist.size(); ++i) {
      const auto& item = WordEnumCtrl_c::namelist[i];
      reserveKeywords_[item] = WordItem_c::make_shared<WordItem_ctrl_c>(WordEnumCtrl_c::toEnum(i));
    }
    // 添加类型
    for (int i = 0; i < WordEnumType_c::namelist.size(); ++i) {
      const auto& item = WordEnumType_c::namelist[i];
      reserveKeywords_[item] = WordItem_c::make_shared<WordItem_type_c>(WordEnumType_c::toEnum(i));
    }
    // 添加内置函数
    for (int i = 0; i < WordEnumNativeCall_c::namelist.size(); ++i) {
      const auto& item = WordEnumNativeCall_c::namelist[i];
      reserveKeywords_[item] =
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
                  (*code_it >= '0' && *code_it <= '9') || '_' == *code_it)) {
            ++code_it;
          }
          end = code_it;
          const auto name_view = std::string_view{start, end};
          WordEnumToken_e token = WordEnumToken_e::Tid;
          // 查找是否已经存在该符号
          auto keywords = reserveKeywords();
          auto result = keywords.find(name_view);
          if (keywords.end() != result) {
            return result->second;
          } else {
            return WordItem_c::make_shared<WordItem_default_c>(WordEnumToken_e::Tid, name_view);
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
            WordLog(Twarning, "数值开头不应使用连续的0；该值将被认为是十进制{}", "");
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
                WordLog(Terror, "预期为8进制的数值，却包含了字符：{}", *code_it);
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
          WordLog(Terror, "字符串缺少右边界：{}", startSign);
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
            WordLog(Terror, "多行注释 /* 缺少右边界");
            return nullptr;
          }
        }
        if (';' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSemicolon);
        }
        if (',' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TComma);
        }
        if ('(' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TLeftCurvesGroup);
        } else if (')' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(
              WordEnumOperator_e::TRightCurvesGroup);
        }
        if ('{' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TLeftFlowerGroup);
        } else if ('}' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(
              WordEnumOperator_e::TRightFlowerGroup);
        }
        if ('[' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TLeftSquareGroup);
        } else if (']' == it) {
          return WordItem_c::make_shared<WordItem_operator_c>(
              WordEnumOperator_e::TRightSquareGroup);
        }
        do {
          if ('|' == it || '&' == it) {
            if (it == *code_it) {
              // || &&
              code_it++;
              if ('|' == it) {
                return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TOr);
              }
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TAnd);
            } else if ('=' == *code_it) {
              // |= &=
              code_it++;
              if ('|' == it) {
                return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSetBitOr);
              }
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSetBitAnd);
            }
            if ('|' == it) {
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TBitOr);
            }
            return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TBitAnd);
          }
          if ('=' == *code_it) {
            // == >= += -= *= /=
            code_it++;
            switch (it) {
            case '=':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TEqual);
            case '>':
              return WordItem_c::make_shared<WordItem_operator_c>(
                  WordEnumOperator_e::TGreaterOrEqual);
            case '<':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TLessOrEqual);
            case '+':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSetAdd);
            case '-':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSetSub);
            case '*':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSetMulti);
            case '/':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSetDivision);
            }
          } else {
            switch (it) {
            case '=':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSet);
            case '>':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TGreater);
            case '<':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TLess);
            case '+':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TAdd);
            case '-':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TSub);
            case '*':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TMulti);
            case '/':
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TDivision);
            }
          }
          if ('?' == it) {
            if ('?' == *code_it) {
              ++code_it;
              if ('=' == *code_it) {
                // ??=
                ++code_it;
                return WordItem_c::make_shared<WordItem_operator_c>(
                    WordEnumOperator_e::TSetNullMerge);
              } else {
                // ??
                return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TNullMerge);
              }
            } else if ('.' == *code_it) {
              // ?.
              ++code_it;
              return WordItem_c::make_shared<WordItem_operator_c>(WordEnumOperator_e::TNullDot);
            }
          }
        } while (false);
        Assert_d(true == false, "未知符号: {}", it);
      }
    }
    return nullptr;
  }

  /**
   * ## 读取一个 [token]
   * - 如果 词法分析失败 或 读取到结尾 则返回 [nullptr]
   */
  std::shared_ptr<WordItem_c> analyse() {
    if (tokenIndex >= 0 && tokenIndex < tokenList.size()) {
      auto result = tokenList[tokenIndex];
      tokenIndex++;
      return result;
    }
    auto result = _innerAnalyse();
    if (nullptr != result) {
      tokenList.push_back(result);
    }
    tokenIndex = tokenList.size();
    return result;
  }
  /**
   * ## 返回当前 [token]
   * - 调用时 [tokenList] 应当非空
   */
  std::shared_ptr<WordItem_c> currentToken() {
    Assert_d(false == tokenList.empty());
    return tokenList.back();
  }

  /**
   * ## 返回指定 [index] 的 [token]
   */
  std::shared_ptr<WordItem_c> getToken(int index) {
    Assert_d(index >= 0 && index < tokenList.size());
    return tokenList[index];
  }

  /**
   * ## 回溯 [token] 读取进度
   */
  void tokenBack(size_t size = 1) { tokenIndex -= size; }

  void debugPrintTokenList() {
    for (const auto& item : tokenList) {
      const auto& word = *item.get();
      word.printInfo();
    }
  }

  const std::map<const std::string_view, std::shared_ptr<WordItem_c>>& reserveKeywords() {
    return reserveKeywords_;
  }

  int current_line = 1;
  std::string_view raw_code;
  const char* code_it = nullptr;
  std::vector<std::shared_ptr<WordItem_c>> tokenList{};
  int tokenIndex = -1;
  std::map<const std::string_view, std::shared_ptr<WordItem_c>> reserveKeywords_;
};