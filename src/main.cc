#include <iostream>

#include "analyse/syntactic_analyse.h"

int main() {
  std::cout << "hello world" << std::endl;
  SyntacticAnalysis_c analyse;
  analyse.init(R"(
 s = "adsf \
122"
  )");
  std::cout << "analyse..." << std::endl;
  analyse.analyse();
  return 0;
}