#include <cassert>
#include <iostream>

#include "lexical_analyse.h"

void test_LexicalAnalyse() {
  LexicalAnalyse_c wordAnalyse;
  wordAnalyse.init(R"(
int a = 10086;
int b  = 0;
int c =  0xf7A0;
int d =  07650;

char ch = 'b';

 // disable=124
 /*disable=uu*/
 /*ll
 disable=bbc*/

s = "sss";
 s = "adsf \
123";
  )");
  while (true) {
    auto word = wordAnalyse.analyse();
    if (nullptr == word) {
      break;
    }
    auto& value = *word.get();
    assert(value.name() != "disable");
    if (WordValueToken_e::Tstring == value.token) {
      assert(value.name() == "adsf \n123" || value.name() == "sss" || value.name() == "b");
    } else if (WordValueToken_e::Tnumber == value.token) {
      auto& num = value.toNumber();
      assert(num.value == 10086 || num.value == 0 || num.value == 0xf7A0 || num.value == 07650);
    } else if (WordValueToken_e::Tundefined == value.token) {
      break;
    }
  }
  wordAnalyse.debugPrintSymbolTable(false);
  std::cout << std::endl << "-----------------------" << std::endl << std::endl;
  wordAnalyse.debugPrintSymbolList();
}

void test_SyntacticAnalysis() {}

int main() {
  std::cout << "<========= test start ========>" << std::endl;
  test_LexicalAnalyse();
  test_SyntacticAnalysis();
  std::cout << "<========= test  end  ========>" << std::endl;
  return 0;
}