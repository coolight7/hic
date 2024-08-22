#pragma once

#include "lexical_analyse.h"

class SyntacticAnalysis_c {
public:
  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

  std::optional<WordItem_c> assertToken(const WordItem_c& limit, bool startWith = false) {
    auto item = lexicalAnalyse.analyse();
    if (item.has_value()) {
      auto& word = item.value();
      if (limit.token == word.token) {
        if ((startWith && word.name.starts_with(limit.name)) || (limit.name == word.name)) {
          return item;
        }
      }
    }
    return std::nullopt;
  }

  std::optional<WordItem_c> assertToken_type(WordValueToken_e type) {
    auto item = lexicalAnalyse.analyse();
    if (item.has_value()) {
      auto& word = item.value();
      if (type == word.token) {
        return item;
      }
    }
    return std::nullopt;
  }

  std::optional<WordItem_c> assertToken_name(const std::string& name, bool startWith = false) {
    auto item = lexicalAnalyse.analyse();
    if (item.has_value()) {
      auto& word = item.value();
      if ((startWith && word.name.starts_with(name)) || (name == word.name)) {
        return item;
      }
    }
    return std::nullopt;
  }

  std::optional<WordItem_c> assertToken_sign(const std::string& sign, bool startWith = false) {
    return assertToken(WordItem_c{WordValueToken_e::Tsign, sign}, startWith);
  }

  bool analyse() {
    while (true) {
      auto item = lexicalAnalyse.analyse();
      if (false == item.has_value()) {
        return true;
      }
      auto& word = item.value();
      switch (word.token) {
      case WordValueToken_e::Ttype:
      case WordValueToken_e::TnativeCall:
      case WordValueToken_e::Tsign:
      case WordValueToken_e::Tkeyword:
      case WordValueToken_e::Tid:
        /* code */
        break;
      default:
        return false;
      }
    }
  }

  LexicalAnalyse_c lexicalAnalyse;
};