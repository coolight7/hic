#pragma once
#include <functional>
#include <memory>

#include "lexical_analyse.h"
#include "magic/macro.h"

#define _GEN_WORD(name)                                                                            \
  if (nullptr == name##_ptr) {                                                                     \
    name##_ptr = lexicalAnalyse.analyse();                                                         \
    if (nullptr == name##_ptr) {                                                                   \
      return nullptr;                                                                              \
    }                                                                                              \
  }                                                                                                \
  auto& name = *name##_ptr.get();

#define _GEN_WORD_DEF(name)                                                                        \
  std::shared_ptr<WordItem_c> name##_ptr;                                                          \
  _GEN_WORD(name)

#define GENERATE_FUN_ITEM_d(...)                                                                   \
  _NameTagConcat_d(_GENERATE_FUN_ITEM, _MacroArgToTag_d(__VA_ARGS__))(__VA_ARGS__)
#define _GENERATE_FUN_ITEM() GENERATE_FUN_ITEM_d
#define _GENERATE_FUN_ITEM1(fun)                                                                   \
  std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<WordItem_c>)>(                         \
      std::bind(&SyntacticAnalysis_c::fun, this, std::placeholders::_1))
#define _GENERATE_FUN_ITEMN(fun, ...)                                                              \
  _GENERATE_FUN_ITEM1(fun), _MacroDefer_d(_GENERATE_FUN_ITEM)()(__VA_ARGS__)

#define _REBACK_d(word_ptr, tempIndex, ...)                                                        \
  reback_funs(word_ptr, tempIndex, _MoreExpand_d(GENERATE_FUN_ITEM_d(__VA_ARGS__)))

class SyntacticAnalysis_c {
public:
  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

  /**
   * ## 回溯依次调用
   */
  template <typename T>
  std::shared_ptr<WordItem_c>
  reback(std::shared_ptr<T>&& word_ptr, int tempIndex,
         std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<T>)>&& fun) {
    lexicalAnalyse.tokenIndex = tempIndex;
    return fun(word_ptr);
  }

  template <typename T, typename... Args>
  std::shared_ptr<WordItem_c>
  reback(std::shared_ptr<T>&& word_ptr, int tempIndex,
         std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<T>)>&& fun,
         std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<Args>)>&&... funlist) {
    auto result =
        reback(std::forward<std::shared_ptr<T>>(word_ptr), tempIndex,
               std::forward<std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<T>)>>(fun));
    if (nullptr != result) {
      return result;
    }
    if constexpr (sizeof...(funlist) > 0) {
      return reback(std::forward<std::shared_ptr<T>>(word_ptr), tempIndex,
                    std::forward<std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<Args>)>>(
                        funlist)...);
    }
    return nullptr;
  }

  template <typename... Args>
  std::shared_ptr<WordItem_c>
  reback_funs(std::shared_ptr<WordItem_c> word_ptr, int tempIndex,
              std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<Args>)>&&... funlist) {
    return reback(std::move(word_ptr), tempIndex, std::move(funlist)...);
  }

  std::shared_ptr<WordItem_c> assertToken(std::shared_ptr<WordItem_c> word_ptr,
                                          const WordItem_c& limit, bool startWith = false) {
    _GEN_WORD(word);
    if (limit.token == word.token) {
      if ((startWith && word.name().starts_with(limit.name())) || (limit.name() == word.name())) {
        return word_ptr;
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_type(std::shared_ptr<WordItem_c> word_ptr,
                                               WordEnumToken_e type) {
    _GEN_WORD(word);
    if (type == word.token) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_name(std::shared_ptr<WordItem_c> word_ptr,
                                               const std::string& name, bool startWith = false) {
    _GEN_WORD(word);
    if ((startWith && word.name().starts_with(name)) || (name == word.name())) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_sign(std::shared_ptr<WordItem_c> word_ptr,
                                               const std::string& sign, bool startWith = false) {
    return assertToken(word_ptr, WordItem_default_c{WordEnumToken_e::Tsign, sign}, startWith);
  }

  // 花括号
  std::shared_ptr<WordItem_c> tryParse_brace(
      std::shared_ptr<WordItem_c> word_ptr,
      const std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<WordItem_c>)>& fun) {
    _GEN_WORD(word)
    const auto left_result = assertToken_sign(word_ptr, "{");
    if (nullptr != left_result) {
      auto result = tryParse_brace(
          nullptr, [&fun](std::shared_ptr<WordItem_c> next_ptr) { return fun(next_ptr); });
      if (nullptr != result) {
        return assertToken_sign(nullptr, "}");
      }
    } else {
      return fun(word_ptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_number_c> parse_constexpr_int(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(word_ptr, WordEnumToken_e::Tnumber);
    if (nullptr != result) {
      return *((std::shared_ptr<WordItem_number_c>*)&result);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_default_c> parse_constexpr_string(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(word_ptr, WordEnumToken_e::Tstring);
    if (nullptr != result) {
      return *((std::shared_ptr<WordItem_default_c>*)&result);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_type(std::shared_ptr<WordItem_c> word_ptr) {
    _GEN_WORD(word);
    if (WordEnumToken_e::Ttype == word.token || WordEnumToken_e::Tid == word.token) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_define(std::shared_ptr<WordItem_c> word_ptr) {
    // value_type
    auto type = parse_value_type(word_ptr);
    if (nullptr != type) {
      // 指针或引用
      _GEN_WORD_DEF(next);
      const auto sign = assertToken_type(next_ptr, WordEnumToken_e::Tsign);
      if (nullptr != sign) {
        if (sign->name() != "*" && sign->name() != "&") {
          return nullptr;
        }
        return next_ptr;
      }
      return type;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_define_id(std::shared_ptr<WordItem_c> word_ptr) {
    // value_type
    auto value_define = parse_value_define(word_ptr);
    if (nullptr != value_define) {
      // ID
      return assertToken_type(value_define, WordEnumToken_e::Tid);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_set_right(std::shared_ptr<WordItem_c> word_ptr) {
    auto sign = assertToken_sign(word_ptr, "=");
    if (nullptr != sign) {
      _GEN_WORD_DEF(next);
      if (WordEnumToken_e::Tnumber == next.token || WordEnumToken_e::Tstring == next.token) {
        return next_ptr;
      }
      if (WordEnumToken_e::Tsign == next.token) {
        if (next.name() != "*" && next.name() != "&") {
          return nullptr;
        } else {
          // 移动 next
          next_ptr = nullptr;
          _GEN_WORD(word);
        }
      }
      if (WordEnumToken_e::Tid == next.token) {
        return next_ptr;
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_set(std::shared_ptr<WordItem_c> word_ptr) {
    auto id = assertToken_type(word_ptr, WordEnumToken_e::Tid);
    if (nullptr != id) {
      if (assertToken_sign(nullptr, "=")) {
        return parse_value_set_right(nullptr);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_define_init(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = parse_value_define_id(word_ptr);
    if (nullptr != result) {
      return parse_value_set_right(nullptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_expr(std::shared_ptr<WordItem_c> word_ptr,
                                         WordEnumType_e ret_type) {
    std::shared_ptr<WordItem_c> last_ptr;
    while (true) {
      _GEN_WORD(word);
      if (word.token == WordEnumToken_e::Tsign && (word.name() == ";" || word.name() == ")")) {
        return last_ptr;
      }
      last_ptr = word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_expr_void(std::shared_ptr<WordItem_c> word_ptr) {
    return parse_expr(word_ptr, WordEnumType_e::Tvoid);
  }

  std::shared_ptr<WordItem_c> parse_code_ctrl_break(std::shared_ptr<WordItem_c> word_ptr) {
    return assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tbreak});
  }

  std::shared_ptr<WordItem_c> parse_code_ctrl_continue(std::shared_ptr<WordItem_c> word_ptr) {
    return assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tcontinue});
  }

  std::shared_ptr<WordItem_c> parse_code_ctrl_return(std::shared_ptr<WordItem_c> word_ptr,
                                                     WordEnumType_e ret_type) {
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Treturn})) {
      return parse_expr(nullptr, ret_type);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_code_ctrl_if(std::shared_ptr<WordItem_c> word_ptr) {
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tif}) &&
        assertToken_sign(nullptr, "(") && parse_expr(nullptr, WordEnumType_e::Tbool) &&
        assertToken_sign(nullptr, ")") && assertToken_sign(nullptr, "{")) {
      _GEN_WORD_DEF(sign);
      if (assertToken_sign(sign_ptr, "}")) {
        // 空代码块 {}
        return sign_ptr;
      }
      if (parse_code(nullptr)) {
        return assertToken_sign(nullptr, "}");
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_code_ctrl_while(std::shared_ptr<WordItem_c> word_ptr) {
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Twhile}) &&
        assertToken_sign(nullptr, "(") && parse_expr(nullptr, WordEnumType_e::Tbool) &&
        assertToken_sign(nullptr, ")") && assertToken_sign(nullptr, "{")) {
      _GEN_WORD_DEF(sign);
      if (assertToken_sign(sign_ptr, "}")) {
        // 空代码块 {}
        return sign_ptr;
      }
      if (parse_code(nullptr)) {
        return assertToken_sign(nullptr, "}");
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_code_ctrl_for(std::shared_ptr<WordItem_c> word_ptr) {
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tfor})) {
      if (assertToken_sign(nullptr, "(")
          // TODO: 匹配中间的 ;; && parse_expr(nullptr)
          && assertToken_sign(nullptr, ")")) {
        if (assertToken_sign(nullptr, "{")) {
          _GEN_WORD_DEF(sign);
          if (assertToken_sign(sign_ptr, "}")) {
            // 空代码块 {}
            return sign_ptr;
          }
          if (parse_code(nullptr)) {
            return assertToken_sign(nullptr, "}");
          }
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_code(std::shared_ptr<WordItem_c> word_ptr) {
    _GEN_WORD(word);
    int tempIndex = lexicalAnalyse.tokenIndex;
    return _REBACK_d(word_ptr, tempIndex, parse_value_define_init, parse_value_set,
                     parse_code_ctrl_if, parse_code_ctrl_while, parse_code_ctrl_for,
                     parse_expr_void);
  }

  std::shared_ptr<WordItem_c> parse_function_call(std::shared_ptr<WordItem_c> word_ptr) {
    auto id_funName = assertToken_type(word_ptr, WordEnumToken_e::Tid);
    if (nullptr != id_funName) {
      if (assertToken_sign(nullptr, "(")) {
        // 参数列表
        std::shared_ptr<WordItem_c> sign_ptr;
        while (assertToken_type(nullptr, WordEnumToken_e::Tid)) {
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(sign_ptr, ",")) {
            break;
          }
        }
        return assertToken_sign(sign_ptr, ")");
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_function_define(std::shared_ptr<WordItem_c> word_ptr) {
    if (parse_value_define(word_ptr)) {
      if (assertToken_type(nullptr, WordEnumToken_e::Tid)) {
        if (assertToken_sign(nullptr, "(")) {
          std::shared_ptr<WordItem_c> sign_ptr;
          while (parse_value_define_id(nullptr)) {
            _GEN_WORD(sign);
            if (nullptr == assertToken_sign(sign_ptr, ",")) {
              break;
            }
          }
          if (assertToken_sign(sign_ptr, ")") && assertToken_sign(nullptr, "{")) {
            auto result = parse_code(nullptr);
            if (nullptr != result) {
              return assertToken_sign(nullptr, "}");
            }
          }
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_function_noReturn_define(std::shared_ptr<WordItem_c> word_ptr) {
    if (assertToken_type(word_ptr, WordEnumToken_e::Tid)) {
      if (assertToken_sign(nullptr, "(")) {
        std::shared_ptr<WordItem_c> sign_ptr;
        while (parse_value_define_id(nullptr)) {
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(sign_ptr, ",")) {
            break;
          }
        }
        if (assertToken_sign(sign_ptr, ")") && assertToken_sign(nullptr, "{")) {
          auto result = parse_code(nullptr);
          if (nullptr != result) {
            return assertToken_sign(nullptr, "}");
          }
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_enum_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken(word_ptr, WordItem_type_c{WordEnumType_e::Tenum});
    if (nullptr != result) {
      if (assertToken_type(nullptr, WordEnumToken_e::Tid) && assertToken_sign(nullptr, "{")) {
        // ID 列表
        std::shared_ptr<WordItem_c> sign_ptr;
        // TODO: 解析 id <= number>?
        while (assertToken_type(nullptr, WordEnumToken_e::Tid)) {
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(sign_ptr, ",")) {
            break;
          }
        }
        return assertToken_sign(sign_ptr, "}");
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_class_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken(word_ptr, WordItem_type_c{WordEnumType_e::Tclass});
    if (nullptr != result) {
      // TODO: class code
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_type_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto enum_ptr = parse_enum_define(word_ptr);
    if (nullptr != enum_ptr) {
      return enum_ptr;
    }
    auto class_ptr = parse_class_define(word_ptr);
    if (nullptr != class_ptr) {
      return class_ptr;
    }
    return nullptr;
  }

  bool analyse() {
    while (true) {
      auto word_ptr = lexicalAnalyse.analyse();
      if (nullptr == word_ptr) {
        return true;
      }
      auto& word = *word_ptr.get();
      if (word.token == WordEnumToken_e::Tsign && (word.name() == ";")) {
        continue;
      }
      auto result = _REBACK_d(word_ptr, lexicalAnalyse.tokenIndex, parse_type_define,
                              parse_value_define_init, parse_function_define);
      if (nullptr == result) {
        return false;
      }
      LexicalAnalyse_c::debugPrintSymbol(*result);
    }
    return true;
  }

  LexicalAnalyse_c lexicalAnalyse;
};