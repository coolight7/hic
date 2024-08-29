#pragma once

#include "lexical_analyse.h"

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

class SyntacticAnalysis_c {
public:
  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

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
        next_ptr = nullptr;
      }
      // ID
      return assertToken_type(next_ptr, WordEnumToken_e::Tid);
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
    auto result = parse_value_define(word_ptr);
    if (nullptr != result) {
      return parse_value_set_right(nullptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_function_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto ret_value = parse_value_define(word_ptr);
    if (nullptr != ret_value) {
      if (assertToken_type(nullptr, WordEnumToken_e::Tid)) {
        if (assertToken_sign(nullptr, "(")) {
          std::shared_ptr<WordItem_c> sign_ptr;
          while (parse_value_define(nullptr)) {
            _GEN_WORD(sign);
            if (nullptr == assertToken_sign(sign_ptr, ",")) {
              break;
            }
          }
          if (assertToken_sign(sign_ptr, ")") && assertToken_sign(nullptr, "{")) {
            // TODO: 函数体 ...
            return assertToken_sign(nullptr, "}");
          }
        }
      }
    }
    return nullptr;
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
      auto result = parse_type_define(word_ptr);
      if (nullptr == result) {
        result = parse_value_define_init(word_ptr);
        if (nullptr == result) {
          result = parse_function_define(word_ptr);
        }
      }
      if (nullptr == result) {
        return false;
      }
      LexicalAnalyse_c::debugPrintSymbol(*result);
    }
    return true;
  }

  LexicalAnalyse_c lexicalAnalyse;
};