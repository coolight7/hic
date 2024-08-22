#include <cassert>
#include <iostream>

#include "word_analyse.h"

void test_word() {
  WordAnalyse_c wordAnalyse;
  wordAnalyse.init(R"(
int a = 10086;
int b  = 0;
int c =  0xf7A0;
int d =  07650;
 s = "adsf \
123";
  )");
  while (true) {
    auto word = wordAnalyse.analyse();
    if (WordValueToken_e::TString == word.token) {
      assert(word.name == "adsf \n123");
    } else if (WordValueToken_e::Tnumber == word.token) {
      assert(word.name == "10086" || word.name == "0" || word.name == std::to_string(0xf7A0) ||
             word.name == std::to_string(07650));
    } else if (WordValueToken_e::TUndefined == word.token) {
      break;
    }
  }
}

int main() {
  std::cout << "<========= test start ========>" << std::endl;
  test_word();
  std::cout << "<========= test  end  ========>" << std::endl;
  return 0;
}