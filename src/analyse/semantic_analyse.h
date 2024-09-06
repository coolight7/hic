#pragma once

// 语义分析

#include "syntactic_analyse.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 `,逗号` 给 UtilLog导致展开异常
 */
#define SemLog(level, tip, ...)                                                                    \
  UtilLog(level, "", syntacticAnalysis.lexicalAnalyse.current_line, tip, ##__VA_ARGS__)

class SymbolItem_c : public ListNode_c {
public:
  SymbolItem_c() : ListNode_c(ListNodeType_e::Symbol) {}

  std::shared_ptr<SyntaxNode_value_define_c> type;
  std::string name;
};

class SemanticAnalyse_c {
public:
  void init(std::string_view in_code) {
    // 添加 global 全局符号表
    symbolTablePush();
    syntacticAnalysis.init(in_code);
  }

  bool checkIdDefine(std::shared_ptr<SymbolItem_c> result) {
    auto& table = currentSymbolTable();
    if (table.find(result->name) != table.end()) {
      // 重定义
      SemLog(Terror, "重定义符号: {}", result->name);
      return false;
    }
    if (nullptr == result->type) {
      // 缺少类型
      SemLog(Terror, "变量声明缺少类型: {}", result->name);
      return false;
    }
    return true;
  }

  std::shared_ptr<SymbolItem_c> checkIdExist(std::shared_ptr<SymbolItem_c> result) {
    return symbolTableFind(result->name);
  }

  bool readNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return false;
    }
    // ...
    bool doPushTable = false;
    switch (node->syntaxType) {
    case SyntaxNodeType_e::Group:
    case SyntaxNodeType_e::ValueDefine: {
    } break;
    case SyntaxNodeType_e::ValueDefineId:
    case SyntaxNodeType_e::ValueDefineInit: {
      // 变量定义，添加符号表
      auto result = std::make_shared<SymbolItem_c>();
      switch (node->syntaxType) {
      case SyntaxNodeType_e::ValueDefineId: {
        auto real_node = HicUtil_c::toType<SyntaxNode_value_define_id_c>(node);
        result->type = real_node->value_define;
        result->name = real_node->id->value;
      } break;
      case SyntaxNodeType_e::ValueDefineInit: {
        auto real_node = HicUtil_c::toType<SyntaxNode_value_define_init_c>(node);
        result->type = real_node->define_id->value_define;
        result->name = real_node->define_id->id->value;
      } break;
      }
      if (false == checkIdDefine(result)) {
        node->debugPrint();
        return false;
      }
      // 添加符号定义
      auto& table = currentSymbolTable();
      table.insert(std::make_pair(result->name, result));
    } break;
    case SyntaxNodeType_e::FunctionCall: {
      // 检查符号是否存在
      auto result = std::make_shared<SymbolItem_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_function_call_c>(node);
      if (real_node->id) {

      }
      auto isExist = checkIdExist(result);
    } break;
    case SyntaxNodeType_e::Operator:
    case SyntaxNodeType_e::CtrlIfBranch:
    case SyntaxNodeType_e::CtrlIf:
    case SyntaxNodeType_e::CtrlWhile:
    case SyntaxNodeType_e::CtrlFor:
    case SyntaxNodeType_e::FunctionDefine: {
      doPushTable = true;
    } break;
    case SyntaxNodeType_e::EnumDefine:
    case SyntaxNodeType_e::ClassDefine: {
      doPushTable = true;
    } break;
    case SyntaxNodeType_e::CtrlReturn:
    }
    if (doPushTable) {
      symbolTablePush();
    }
    for (auto& item : node->children) {
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
      } break;
      case ListNodeType_e::Syntactic: {
        // 递归读取
        auto result = readNode(HicUtil_c::toType<SyntaxNode_c>(item));
        if (false == result) {
          return false;
        }
      } break;
      case ListNodeType_e::Symbol:
        return false;
      }
    }
    if (doPushTable) {
      symbolTablePop();
    }
    return true;
  }

  bool analyse() {
    // 语法分析
    auto syntactic_result = syntacticAnalysis.analyse();
    if (false == syntactic_result) {
      return false;
    }
    // 语义分析
    return readNode(syntacticAnalysis.root);
  }

  std::map<std::string, std::shared_ptr<SymbolItem_c>>& globalSymbolTable() {
    Assert_d(symbolTable.empty() == false);
    return symbolTable.front();
  }

  std::map<std::string, std::shared_ptr<SymbolItem_c>>& currentSymbolTable() {
    Assert_d(symbolTable.empty() == false);
    return symbolTable.back();
  }

  void symbolTablePush() { symbolTable.emplace_back(); }

  void symbolTablePop() { symbolTable.pop_back(); }

  // 在 [symbolTable] 中查找符号，且是从最近/最小的作用域开始查找
  std::shared_ptr<SymbolItem_c> symbolTableFind(const std::string& key) {
    if (false == symbolTable.empty()) {
      for (auto it = symbolTable.end() - 1;; --it) {
        if (it->contains(key)) {
          return (*it)[key];
        }
        if (it == symbolTable.begin()) {
          break;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SymbolItem_c> globalSymbolTableFind(const std::string& key) {
    auto& curr = globalSymbolTable();
    if (curr.contains(key)) {
      return curr[key];
    }
    return nullptr;
  }

  std::shared_ptr<SymbolItem_c> currentSymbolTableFind(const std::string& key) {
    auto& curr = currentSymbolTable();
    if (curr.contains(key)) {
      return curr[key];
    }
    return nullptr;
  }

  void debugPrintSymbolTable() {
    int i = 0;
    for (const auto& table : symbolTable) {
      std::cout << i + 1 << std::endl;
      for (const auto& it : table) {
        for (auto j = i; j-- > 0;) {
          std::cout << "  ";
        }
        std::cout << "- " << it.second->name << " : " << std::endl;
        it.second->type->debugPrint();
      }
    }
  }

  // 作用域符号表
  std::vector<std::map<std::string, std::shared_ptr<SymbolItem_c>>> symbolTable{};
  SyntacticAnalysis_c syntacticAnalysis{};
};