#include <iostream>

#include "word_analyse.h"

int main() {
  std::cout << "hello world" << std::endl;
  WordAnalyse_c wordAnalyse;
  wordAnalyse.init(R"(
 s = "adsf"
  )");
  std::cout << "analyse..." << std::endl;
  while (true) {
    auto word = wordAnalyse.analyse();
    if (WordValueToken_e::TUndefined == word.token) {
      wordAnalyse.debugPrint();
      return -1;
    }
  }
  return 0;
}