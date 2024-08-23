#pragma once

#include "lexical_analyse.h"

class SyntacticAnalysis_c {
public:
  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

  std::shared_ptr<WordItem_c> assertToken(const WordItem_c& limit, bool startWith = false) {
    auto item = lexicalAnalyse.analyse();
    if (nullptr != item) {
      auto& word = *item.get();
      if (limit.token == word.token) {
        if ((startWith && word.name().starts_with(limit.name())) || (limit.name() == word.name())) {
          return item;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_type(WordEnumToken_e type) {
    auto item = lexicalAnalyse.analyse();
    if (nullptr != item) {
      auto& word = *item.get();
      if (type == word.token) {
        return item;
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_name(const std::string& name, bool startWith = false) {
    auto item = lexicalAnalyse.analyse();
    if (nullptr != item) {
      auto& word = *item.get();
      if ((startWith && word.name().starts_with(name)) || (name == word.name())) {
        return item;
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_sign(const std::string& sign, bool startWith = false) {
    return assertToken(WordItem_default_c{WordEnumToken_e::Tsign, sign}, startWith);
  }

  bool parse_const() { return true; }

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
        while (nullptr != assertToken_sign("*")) {
        }
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