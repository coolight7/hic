#include <iostream>

#include "syntactic_analysis.h"

int main() {
  std::cout << "hello world" << std::endl;
  SyntacticAnalysis_c analyse;
  analyse.init(R"(
 s = "adsf \
122"
  )");
  std::cout << "analyse..." << std::endl;
  analyse.analyse();
  analyse.lexicalAnalyse.debugPrint(false);
  return 0;
}