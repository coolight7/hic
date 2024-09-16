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

#include "entity.h"
#include "rule.h"
#include "src/util.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 `,逗号` 给 UtilLog导致展开异常
 */
#define WordLog(level, tip, ...) UtilLineLog(level, "", current_line, tip, ##__VA_ARGS__)

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

  bool init(std::string_view in_code) {
    Assert_d(false == in_code.empty(), "代码不应为空");
    if (in_code.empty()) {
      return false;
    }
    raw_code = in_code;
    code_it = raw_code.begin();
    current_line = 1;
    tokenList.clear();
    tokenIndex = -1;
    reserveKeywords_.clear();
    // 添加控制关键字
    for (int i = 0; i < WordEnumCtrl_c::namelist.size(); ++i) {
      const auto& item = WordEnumCtrl_c::namelist[i];
      reserveKeywords_[item] = WordItem_c::make_shared<WordItem_ctrl_c>(WordEnumCtrl_c::toEnum(i));
    }
    // 添加类型
    for (int i = 0; i < WordEnumType_c::namelist.size(); ++i) {
      const auto& item = WordEnumType_c::namelist[i];
      reserveKeywords_[item] = WordItem_c::make_shared<WordItem_type_c>(WordEnumType_c::toEnum(i));
    }
    // 添加内置值
    for (int i = 0; i < WordEnumValue_c::namelist.size(); ++i) {
      const auto& item = WordEnumValue_c::namelist[i];
      reserveKeywords_[item] =
          WordItem_c::make_shared<WordItem_value_c>(WordEnumValue_c::toEnum(i));
    }
    // 添加内置函数
    for (int i = 0; i < WordEnumNativeCall_c::namelist.size(); ++i) {
      const auto& item = WordEnumNativeCall_c::namelist[i];
      reserveKeywords_[item] =
          WordItem_c::make_shared<WordItem_nativeCall_c>(WordEnumNativeCall_c::toEnum(i));
    }
    return true;
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
          // 查找是否已经存在该符号
          auto keywords = reserveKeywords();
          auto result = keywords.find(name_view);
          if (keywords.end() != result) {
            return result->second;
          } else {
            return WordItem_c::make_shared<WordItem_id_c>(name_view);
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
          // TODO: 0b 二进制
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
          return WordItem_c::make_shared<WordItem_string_c>(value);
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
            // TODO： ++ --
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

#undef WordLog