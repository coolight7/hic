#pragma once

// 汇编生成

#include "semantic_analyse.h"

class GenerateAsm_c {
public:
  inline static bool enableLog_genNode = false;

  bool init(std::string_view in_code) {
    code = "";
    return semanticAnalyse.init(in_code);
  }

  template <typename... _ARGS> bool genNodeList(std::shared_ptr<_ARGS>... args) {
    return (genNode(args) && ...);
  }

  bool genNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return false;
    }
    if (enableLog_genNode) {
      std::cout << "## AnalyseNode: ------ " << std::endl;
      node->debugPrint();
    }
    return true;
  }

  bool generate() {
    // 语义分析
    if (false == semanticAnalyse.analyse()) {
      return "";
    }
    // 遍历生成
    return genNode(semanticAnalyse.tree());
  }

  std::string code;
  SemanticAnalyse_c semanticAnalyse;
};