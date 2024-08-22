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
    if (false == word.has_value()) {
      break;
    }
    auto& value = word.value();
    std ::cout << value.token << "\t" << value.name << std::endl;
  }
  wordAnalyse.debugPrint(false);
  return 0;
}