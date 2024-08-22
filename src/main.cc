#include <iostream>

#include "word_analyse.h"

int main() {
  std::cout << "hello world" << std::endl;
  WordAnalyse_c wordAnalyse;
  wordAnalyse.init(R"(
 s = "adsf \
122"
  )");
  std::cout << "analyse..." << std::endl;
  while (true) {
    auto word = wordAnalyse.analyse();
    std ::cout << word.token << "\t" << word.name << std::endl;
    if (WordValueToken_e::TUndefined == word.token) {
      break;
    }
  }
  wordAnalyse.debugPrint();
  return 0;
}