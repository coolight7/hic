#include <iostream>

#include "source/test.h"
#include "src/analyse/syntactic_analyse.h"

int main() {
  std::cout << "hello world" << std::endl;
  std::cout << "add: " << test_add(1, 2) << std::endl;
  SyntacticAnalysis_c analyse;
  analyse.init(R"(
 s = "adsf \
122"
  )");
  std::cout << "analyse..." << std::endl;
  analyse.analyse();
  return 0;
}