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
public:
  void debugPrint() {}

  std::list<ProgramSegment_c> segments{};
};

// 程序包
class ProgramPackage_c {
public:
  void init() {
    data = "";
    code = "";
  }

  template <typename T> T* hitData(int addr) {
    Assert_d(addr >= 0 && addr < data.size(), addr);
    return (T*)(data.c_str() + addr);
  }

  void debugPrint() {
    std::cout << std::endl << "vvv ----------- <header> ----------- vvv" << std::endl;
    header.debugPrint();
    std::cout << std::endl << "^^^ ----------- <header> ----------- ^^^" << std::endl;
    std::cout << "<data> size: " << data.size() << std::endl;
    std::cout << "<code> size: " << code.size() << std::endl;
  }

  // 程序头
  ProgramHeader_c header{};
  // 数据指令
  std::string data{};
  // 二进制指令
  std::string code{};
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
      auto real_node = HicUtil_c::toType<SyntaxNode_value_define_init_c>(node);
      if (false == symbolManager->currentIsGlobal()) {
        // 添加初始化指令
        auto data = real_node->data;
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
    auto root = semanticAnalyse.tree();
    // 分配全局变量内存
    for (const auto& item : (*root->symbolTable)) {
      switch (item.second->symbolType) {
      case SymbolType_e::TValue: {
        // 分配地址
        auto symbol = SymbolItem_c::toValue(item.second);
        symbol->address = program.data.size();
        program.data.resize(program.data.size() + symbol->size());
        // 简单赋值
        *(program.hitData<int>(symbol->address)) = int(symbol->value);
        // 函数调用
      } break;
      case SymbolType_e::TEnum: {
        // 不用操作
      } break;
      default:
        break;
      }
    }
    // 遍历生成
    return genNode(root);
  }

  ProgramPackage_c program{};
  SemanticAnalyse_c semanticAnalyse{};
  std::shared_ptr<SymbolManager_c> symbolManager;
};