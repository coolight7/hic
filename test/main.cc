#include <fstream>
#include <iostream>

#include "analyse/lexical_analyse.h"
#include "analyse/semantic_analyse.h"
#include "analyse/syntactic_analyse.h"

void test_LexicalAnalyse() {
  std::cout << std::endl << "----------- test_LexicalAnalyse -----------" << std::endl << std::endl;
  LexicalAnalyse_c wordAnalyse{};
  wordAnalyse.init(R"(
int a = 10086;
int b  = 0;
int c =  0xf7A0;
int d =  07650;

int main() {
  d += b;
  c -= b;
  d /= b;
  c *= b;
  a ??= b ?? c;

  if (a == b || (b == c && c == d)) {
      d = b;
  }

  char ch = 'b';

      // disable=124
  bool k = false;
  // disable = fdsa
  bool g = false;
  /*disable=uu*/
  /*ll
  disable=bbc*/

  s = "sss";
  s = "adsf \
  123";
  return 0;
}
  )");
  while (true) {
    auto word = wordAnalyse.analyse();
    if (nullptr == word) {
      break;
    }
    auto& value = *word.get();
    Assert_d(value.name() != "disable", value.name());
    if (WordEnumToken_e::Tstring == value.token) {
      Assert_d(value.name() == "adsf \n  123" || value.name() == "sss" || value.name() == "b",
               value.name());
    } else if (WordEnumToken_e::Tnumber == value.token) {
      auto& num = value.toNumber();
      Assert_d(num.value == 10086 || num.value == 0 || num.value == 0xf7A0 || num.value == 07650,
               num.value);
    } else if (WordEnumToken_e::Tundefined == value.token) {
      break;
    }
  }
  std::cout << std::endl << "-----------------------" << std::endl << std::endl;
  wordAnalyse.debugPrintTokenList();
}

void test_SyntacticAnalysis() {
  std::cout << std::endl
            << "----------- test_SyntacticAnalysis -----------" << std::endl
            << std::endl;
  SyntacticAnalysis_c analyse{};
  analyse.init(R"(
int a = 10086;
int b = 0;
int c = 0xf7A0;
int d = 07650;
int* f = &d;
String str = "qiqi";

int test(int a, int b) {
  return (a + b);
}

enum Hello_e {
  A1,
  B2,
  CC
};

int main(String* args, int size) {
    d += b;
    c -= b;
    d /= b;
    c *= b;
    a ??= b ?? c;

    int ret = test(1, 2);
    test(3, 4);
    test(3,);
    test();
    if (a == b || (b == c && a == c)) {
        d = b;
    }

    int ok = *c;
    char ch = 'b';
        // disable=124
    bool k = false;
    // disable = fdsa
    bool g = false;
    /*disable=uu*/
    /*ll
    disable=bbc*/

    s = "sss";
    s = "adsf 123";
    return 0;
}
  )");
  auto rebool = analyse.analyse();
  if (false == rebool) {
    int index = analyse.lexicalAnalyse.tokenList.size();
    if (index > 10) {
      index -= 10;
    } else {
      index = 0;
    }
    UtilLineLog(Tdebug, "", 0, "## End last Token:");
    for (; index < analyse.lexicalAnalyse.tokenList.size(); ++index) {
      auto& item = analyse.lexicalAnalyse.tokenList[index];
      item->printInfo();
    }
  }
  UtilLineLog(Tdebug, "", 0, "## tree:");
  analyse.root->debugPrint();
  assert(rebool);
}

void test_SemanticAnalyse() {
  SyntacticAnalysis_c::enableLog_assertToken = false;
  SyntacticAnalysis_c::enableLog_parseCode = false;
  SemanticAnalyse_c::enableLog_analyseNode = true;
  std::cout << std::endl
            << "----------- test_SemanticAnalyse -----------" << std::endl
            << std::endl;
  SemanticAnalyse_c analyse{};
  analyse.init(R"(
int a = 10086;
int b = 0;
int c = 0xf7A0;
int d = 07650;
int* f = &d;
String str = "qiqi";

int test(int& a, int** &b) {
  return (a + b);
}

enum Hello_e {
  A1,
  B2,
  CC
};

int main(char** args, int size) {
    d += b;
    c -= b;
    d /= b;
    c *= b;
    a ??= b ?? c;

    int ret = test(1, 2);
    test(3, 4);
    test(ret, 4, );  // 允许尾部多余的 ,
    // test(3,);     // 检查函数参数个数
    // test();
    // test_undef(); // 检查未定义符号
    if (a == b || (b == c && a == c)) {
        d = b;
    }

    int ok = *c;
    char ch = 'b';
        // disable=124
    bool k = false;
    // disable = fdsa
    bool g = false;
    /*disable=uu*/
    /*ll
    disable=bbc*/

    s = "sss";
    s = "adsf 123";
    return 0;
}
  )");
  auto rebool = analyse.analyse();
  if (false == rebool) {
    int index = analyse.syntacticAnalysis.lexicalAnalyse.tokenList.size();
    if (index > 10) {
      index -= 10;
    } else {
      index = 0;
    }
    UtilLineLog(Tdebug, "", 0, "## End last Token:");
    for (; index < analyse.syntacticAnalysis.lexicalAnalyse.tokenList.size(); ++index) {
      auto& item = analyse.syntacticAnalysis.lexicalAnalyse.tokenList[index];
      item->printInfo();
    }
  } else {
    UtilLineLog(Tdebug, "", 0, "## tree:");
    analyse.syntacticAnalysis.root->debugPrint();
  }
  assert(rebool);
}

#ifndef PROGEAM_ROOT_DIR
#define PROGEAM_ROOT_DIR
#endif

void test_read() {
  std::cout << std::endl << "----------- test_read -----------" << std::endl << std::endl;
  std::string code;
  const char* file_path = PROGEAM_ROOT_DIR "/resource/test1.hic";
  std::ifstream stream{file_path};
  if (false == stream.is_open()) {
    std::cout << "文件打开失败: " << file_path << std::endl;
    return;
  }
  for (std::string line; getline(stream, line);) {
    code += line;
    code += "\n";
  }
  stream.close();

  SyntacticAnalysis_c::enableLog_assertToken = false;
  SyntacticAnalysis_c::enableLog_parseCode = false;
  SemanticAnalyse_c::enableLog_analyseNode = true;
  SemanticAnalyse_c analyse{};
  analyse.init(code);
  auto rebool = analyse.analyse();
  if (false == rebool) {
    int index = analyse.syntacticAnalysis.lexicalAnalyse.tokenList.size();
    if (index > 10) {
      index -= 10;
    } else {
      index = 0;
    }
    UtilLineLog(Tdebug, "", 0, "## End last Token:");
    for (; index < analyse.syntacticAnalysis.lexicalAnalyse.tokenList.size(); ++index) {
      auto& item = analyse.syntacticAnalysis.lexicalAnalyse.tokenList[index];
      item->printInfo();
    }
  } else {
    UtilLineLog(Tdebug, "", 0, "## tree:");
    analyse.syntacticAnalysis.root->debugPrint();
  }
  assert(rebool);
}

int main() {
  std::cout << "<========= test start ========>" << std::endl;
  test_LexicalAnalyse();
  test_SyntacticAnalysis();
  test_SemanticAnalyse();
  test_read();
  std::cout << "<========= test  end  ========>" << std::endl;
  return 0;
}