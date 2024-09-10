#pragma once

// 汇编生成

#include "semantic_analyse.h"

// 节
class ProgramSection_c {
public:
  size_t start = 0;
  size_t size = 0;
};

// 段
class ProgramSegment_c {
public:
  std::list<ProgramSection_c> sections{};
};

// 程序头
class ProgramHeader_c {
  std::list<ProgramSegment_c> segments{};
};

// 程序包
class ProgramPackage_c {
public:
  void init() { code = ""; }

  // 二进制指令
  std::string code{};
  // 程序头
  ProgramHeader_c header{};
};

class GenerateAsm_c {
public:
  inline static bool enableLog_genNode = false;

  bool init(std::string_view in_code) {
    program.init();
    auto result = semanticAnalyse.init(in_code);
    symbolManager = semanticAnalyse.symbolManager;
    return result;
  }

  template <typename... _ARGS> bool tryGenNodeList(std::shared_ptr<_ARGS>... args) {
    return (tryGenNode(args) && ...);
  }

  template <typename... _ARGS> bool genNodeList(std::shared_ptr<_ARGS>... args) {
    return (genNode(args) && ...);
  }

  bool tryGenNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return true;
    }
    return genNode(node);
  }

  template <typename... _Args> bool genChildren(std::shared_ptr<SyntaxNode_c> node) {
    // 读取子节点
    int index = 0;
    for (auto& item : node->children) {
      index++;
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
      } break;
      case ListNodeType_e::Syntactic: {
        auto real_node = genNode(HicUtil_c::toType<SyntaxNode_c>(item));
        if (false == real_node) {
          return false;
        }
      } break;
      case ListNodeType_e::Symbol: {
        UtilLog(Terror, "{} 非预期的子节点类型为 符号表项 {}/{}", node->name(), index,
                node->children.size());
        return false;
      }
      }
    }
    return true;
  }

  // 生成汇编
  bool genNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return false;
    }
    if (enableLog_genNode) {
      std::cout << "## GenNode: ------ " << std::endl;
      node->debugPrint();
    }
    // 记录当前符号表深度，后续恢复
    int symbolTableDeep = symbolManager->stack.size();
    switch (node->syntaxType) {
    case SyntaxNodeType_e::TValueDefineId:
    case SyntaxNodeType_e::TValueDefine:
      break;
    case SyntaxNodeType_e::TNormal: {
      if (false == genChildren(node)) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TGroup: {
      // {} 隔离符号范围
      auto real_node = HicUtil_c::toType<SyntaxNode_group_c>(node);
      symbolManager->push(real_node.get());
      if (false == genChildren(node)) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TValueDefineInit: {
      // 全局区的符号由程序启动初始化，这里只初始化局部变量
      if (false == symbolManager->currentIsGlobal()) {
        
      }
    } break;
    default:
      break;
    }

    // 恢复符号表层级
    if (symbolTableDeep != symbolManager->stack.size()) {
      symbolManager->pop();
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

  ProgramPackage_c program{};
  SemanticAnalyse_c semanticAnalyse{};
  std::shared_ptr<SymbolManager_c> symbolManager;
};