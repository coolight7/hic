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
 // disable=124
 /*disable=uu*/
 /*ll
 disable=bbc*/
 s = "adsf \
123";
  )");
  while (true) {
    auto word = wordAnalyse.analyse();
    if (false == word.has_value()) {
      break;
    }
    auto value = word.value();
    assert(value.name != "disable");
    if (WordValueToken_e::Tstring == value.token) {
      assert(value.name == "adsf \n123");
    } else if (WordValueToken_e::Tnumber == value.token) {
      assert(value.name == "10086" || value.name == "0" || value.name == std::to_string(0xf7A0) ||
             value.name == std::to_string(07650));
    } else if (WordValueToken_e::Tundefined == value.token) {
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