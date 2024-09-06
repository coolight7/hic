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

  // 检查函数类型
  template <typename... _ARGS>
  bool checkFunType(const SyntaxNode_value_define_c&& return_type, const _ARGS&&... args) {
    return true;
  }

  // 检查变量类型
  // - 对象自身为已定义符号
  // - [in_type] 为引用符号类型需求
  bool checkValueType(const SyntaxNode_value_define_c&& in_type) {
    if (type->value_type->token != in_type.value_type->token) {
      return false;
    }
    if (type->pointer != in_type.pointer) {
      // 检查指针层级
      // 不用检查引用 type->isReferer != in_type.isReferer
      return false;
    }
    switch (type->value_type->token) {
    case WordEnumToken_e::Tid: {
      // 自定义类型
      // 类型名称
      return (type->value_type->toDefault().value == in_type.value_type->toDefault().value);
    } break;
    case WordEnumToken_e::Ttype: {
      // 内置类型
      return (type->value_type->toType().value == in_type.value_type->toType().value);
    } break;
    default:
      Assert_d(true == false, "{} 非法的变量类型: {}", name,
               WordEnumToken_c::toName(type->value_type->token));
      return false;
      break;
    }
  }

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

  bool checkIdDefine(std::shared_ptr<SymbolItem_c> symbol) {
    Assert_d(false == symbol->name.empty(), "符号名称不应为空");
    auto& table = currentSymbolTable();
    if (table.find(symbol->name) != table.end()) {
      // 重定义
      SemLog(Terror, "重定义符号: {}", symbol->name);
      return false;
    }
    if (nullptr == symbol->type) {
      // 缺少类型
      SemLog(Terror, "符号声明缺少类型: {}", symbol->name);
      return false;
    }
    return true;
  }

  std::shared_ptr<SymbolItem_c> checkIdExist(std::shared_ptr<SymbolItem_c> symbol) {
    Assert_d(false == symbol->name.empty(), "符号名称不应为空");
    auto result = symbolTableFind(symbol->name);
    if (nullptr == symbol) {
      SemLog(Terror, "未定义符号: {}", symbol->name);
    }
    return result;
  }

  bool analyseNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return false;
    }
    // ...
    int symbolTableDeep = symbolTable.size();
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
      result->name = real_node->id->toDefault().value;
      // 检查符号定义
      auto exist_id = checkIdExist(result);
      if (nullptr == exist_id) {
        return false;
      }
      // 检查函数类型匹配
      //   if (exist_id->checkFunType()) {

      //   }
    } break;
    case SyntaxNodeType_e::FunctionDefine: {
      symbolTablePush();
      // 检查定义
      auto result = std::make_shared<SymbolItem_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_function_define_c>(node);
      result->name = real_node->id->toDefault().value;
      result->type = real_node->id;
      if (false == checkIdDefine(result)) {
        return false;
      }
      // 读取 args
      for (auto item : real_node->args) {
        if (false == analyseNode(item)) {
          return false;
        }
      }
      // 读取body
      if (false == analyseNode(real_node->body)) {
        return false;
      }
      // 添加符号定义
      auto& table = currentSymbolTable();
      table.insert(std::make_pair(result->name, result));
    } break;
    case SyntaxNodeType_e::CtrlIfBranch:
    case SyntaxNodeType_e::CtrlIf:
    case SyntaxNodeType_e::CtrlWhile:
    case SyntaxNodeType_e::CtrlFor: {
      symbolTablePush();
    } break;
    case SyntaxNodeType_e::EnumDefine:
    case SyntaxNodeType_e::ClassDefine: {
      symbolTablePush();
    } break;
    case SyntaxNodeType_e::Operator:
    case SyntaxNodeType_e::CtrlReturn:
      break;
    }
    // 递归读取子节点
    for (auto& item : node->children) {
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
      } break;
      case ListNodeType_e::Syntactic: {
        auto result = analyseNode(HicUtil_c::toType<SyntaxNode_c>(item));
        if (false == result) {
          return false;
        }
      } break;
      case ListNodeType_e::Symbol:
        return false;
      }
    }
    if (symbolTableDeep != symbolTable.size()) {
      // 恢复符号表层级
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
    return analyseNode(syntacticAnalysis.root);
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