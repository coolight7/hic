#pragma once

#include "lexical_analyse.h"

#define _GEN_WORD(name)                                                                            \
  if (nullptr == name##_ptr) {                                                                     \
    name##_ptr = lexicalAnalyse.analyse();                                                         \
    if (nullptr == name##_ptr) {                                                                   \
      return name##_ptr;                                                                           \
    }                                                                                              \
  }                                                                                                \
  auto& name = *name##_ptr.get();

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
      return static_cast<std::shared_ptr<WordItem_number_c>>(result);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_constexpr_string(std::shared_ptr<WordItem_c> word_ptr) {
    return assertToken_type(word_ptr, WordEnumToken_e::Tstring);
  }

  std::shared_ptr<WordItem_c> parse_value_type(std::shared_ptr<WordItem_c> word_ptr) {
    _GEN_WORD(word);
    if (WordEnumToken_e::Ttype == word.token) {
    }
  }

  bool analyse() {
    while (true) {
      auto word_ptr = lexicalAnalyse.analyse();
      if (nullptr == word_ptr) {
        return true;
      }
      auto& word = *word_ptr.get();
      switch (word.token) {
      case WordEnumToken_e::Tkeyword: {
        auto& item = word.toKeyword();
        switch (item.value) {
        case WordEnumCtrl_e::Tconst: {
        } break;
        case WordEnumCtrl_e::Tstatic: {
        } break;
        default:
          break;
        }
      } break;
      case WordEnumToken_e::Ttype: {
        // 类型符
        auto& item = word.toType();
      } break;
      case WordEnumToken_e::TnativeCall: {
        auto& item = word.toNativeCall();
      } break;
      case WordEnumToken_e::Tsign: {
        auto& item = word.toDefault();
      } break;
      case WordEnumToken_e::Tid: {
        auto& item = word.toDefault();
      } break;
      default:
        return false;
      }
    }
  }

  LexicalAnalyse_c lexicalAnalyse;
};