#pragma once

#include "lexical_analyse.h"

class SyntacticAnalysis_c {
public:
  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

  bool analyse() {
    while (true) {
      auto item = lexicalAnalyse.analyse();
      if (false == item.has_value()) {
        return true;
      }
      auto& word = item.value();
      std ::cout << word.token << "\t" << word.name << std::endl;
    }
  }

  LexicalAnalyse_c lexicalAnalyse;
};