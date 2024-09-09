#pragma once

// 语义分析

#include "syntactic_analyse.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 `,逗号` 给 UtilLog导致展开异常
 */
#define SemLog(level, tip, ...) UtilLog(level, tip, ##__VA_ARGS__)

class SymbolItem_value_c;
class SymbolItem_function_c;

/**
 * 符号表项
 */
class SymbolItem_c : public ListNode_c {
public:
  SymbolItem_c(SymbolType_e in_symbolType)
      : ListNode_c(ListNodeType_e::Symbol), symbolType(in_symbolType) {}

  SymbolItem_c(const SymbolItem_c&) = delete;

  virtual bool hasType() const { return true; }

  inline static std::shared_ptr<SymbolItem_value_c> toValue(std::shared_ptr<SymbolItem_c> ptr) {
    if (nullptr != ptr) {
      switch (ptr->symbolType) {
      case SymbolType_e::TValue: {
        return HicUtil_c::toType<SymbolItem_value_c>(ptr);
      } break;
      case SymbolType_e::TFunction: {
      } break;
      }
    }
    return nullptr;
  }

  inline static std::shared_ptr<SymbolItem_function_c>
  toFunction(std::shared_ptr<SymbolItem_c> ptr) {
    if (nullptr != ptr) {
      switch (ptr->symbolType) {
      case SymbolType_e::TValue: {
      } break;
      case SymbolType_e::TFunction: {
        return HicUtil_c::toType<SymbolItem_function_c>(ptr);
      } break;
      }
    }
    return nullptr;
  }

  virtual void debugPrint() {
    std::cout << SymbolType_c::toName(symbolType) << " : " << name << std::endl;
  }

  SymbolType_e symbolType;
  std::string name;
};

// 变量符号定义
class SymbolItem_value_c : public SymbolItem_c {
public:
  SymbolItem_value_c() : SymbolItem_c(SymbolType_e::TValue) {}

  virtual void debugPrint() {
    std::cout << SymbolType_c::toName(symbolType) << " : " << name << std::endl;
  }

  bool hasType() const override { return (nullptr != type); }

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
      return (type->value_type->toId().value == in_type.value_type->toId().value);
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
  std::list<std::shared_ptr<WordItem_c>> refs;
};

// 函数符号定义
class SymbolItem_function_c : public SymbolItem_c {
public:
  SymbolItem_function_c() : SymbolItem_c(SymbolType_e::TFunction) {}

  bool hasType() const override { return (nullptr != type); }

  // 检查函数类型
  template <typename... _ARGS>
  bool checkFunType(const SyntaxNode_value_define_c&& return_type, const _ARGS&&... args) {
    return true;
  }

  // TODO: 检查传入参数是否匹配
  bool checkFunArgs(std::list<std::shared_ptr<ListNode_c>>& args) {
    if (args.size() != type->args.size()) {
      SemLog(Terror, "函数参数个数不匹配: {} (预期 {} 个，但给定了 {} 个)", name, type->args.size(),
             args.size());
      return false;
    }
    return true;
  }

  std::shared_ptr<SyntaxNode_function_define_c> type;
  std::list<std::shared_ptr<SyntaxNode_function_call_c>> refs;
};

// 枚举符号定义
class SymbolItem_enum_c : public SymbolItem_c {
public:
  SymbolItem_enum_c() : SymbolItem_c(SymbolType_e::TEnum) {}

  bool hasType() const override { return (nullptr != type); }

  std::shared_ptr<SyntaxNode_enum_define_c> type;
};

class SemanticAnalyse_c {
public:
  inline static bool enableLog_analyseNode = false;

  bool init(std::string_view in_code) {
    symbolTableStack.clear();
    // 添加 global 全局符号表
    symbolTablePush();
    return syntacticAnalysis.init(in_code);
  }

  bool checkIdDefine(std::shared_ptr<SymbolItem_c> symbol) {
    Assert_d(nullptr != symbol, "符号不应为 nullptr");
    Assert_d(false == symbol->name.empty(), "符号名称不应为空");
    auto& table = currentSymbolTable();
    if (table->find(symbol->name) != table->end()) {
      // 重定义
      SemLog(Terror, "重定义符号: {}", symbol->name);
      return false;
    }
    if (false == symbol->hasType()) {
      // 缺少类型
      SemLog(Terror, "符号声明缺少类型: {}", symbol->name);
      return false;
    }
    return true;
  }

  std::shared_ptr<SymbolItem_c> checkIdExist(std::shared_ptr<SymbolItem_c> symbol) {
    Assert_d(nullptr != symbol, "符号不应为 nullptr");
    Assert_d(false == symbol->name.empty(), "符号名称不应为空");
    auto result = symbolTableFind(symbol->name);
    if (nullptr == result) {
      SemLog(Terror, "未定义符号: {}", symbol->name);
    }
    return result;
  }

  template <typename... _ARGS> bool analyseNodeList(std::shared_ptr<_ARGS>... args) {
    return (analyseNode(args) && ...);
  }

  template <typename... _ARGS> bool tryAnalyseNodeList(std::shared_ptr<_ARGS>... args) {
    return (tryAnalyseNode(args) && ...);
  }

  bool tryAnalyseNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return true;
    }
    return analyseNode(node);
  }

  /**
   * ## 解析节点
   * -
   */
  bool analyseNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return false;
    }
    // ...
    if (enableLog_analyseNode) {
      std::cout << "## AnalyseNode: ------ " << std::endl;
      node->debugPrint();
    }
    int symbolTableDeep = symbolTableStack.size();
    // [1]
    switch (node->syntaxType) {
    case SyntaxNodeType_e::TNormal:
    case SyntaxNodeType_e::TValueDefine: {
    } break;
    case SyntaxNodeType_e::TGroup: {
      // {} 隔离符号范围
      auto real_node = HicUtil_c::toType<SyntaxNode_group_c>(node);
      symbolTablePush(real_node.get());
    } break;
    case SyntaxNodeType_e::TValueDefineId:
    case SyntaxNodeType_e::TValueDefineInit: {
      // 变量定义，添加符号表
      auto result = std::make_shared<SymbolItem_value_c>();
      switch (node->syntaxType) {
      case SyntaxNodeType_e::TValueDefineId: {
        auto real_node = HicUtil_c::toType<SyntaxNode_value_define_id_c>(node);
        result->type = real_node->value_define;
        result->name = real_node->id->id;
        real_node->symbol = result;
      } break;
      case SyntaxNodeType_e::TValueDefineInit: {
        auto real_node = HicUtil_c::toType<SyntaxNode_value_define_init_c>(node);
        result->type = real_node->define_id->value_define;
        result->name = real_node->define_id->id->id;
        real_node->define_id->symbol = result;
      } break;
      default: {
        Assert_d(true == false, "非预期的变量定义节点：{}",
                 SyntaxNodeType_c::toName(node->syntaxType));
      } break;
      }
      // 添加符号定义
      if (false == currentAddSymbol(result)) {
        node->debugPrint();
        return false;
      }
    } break;
    case SyntaxNodeType_e::TFunctionCall: {
      // 检查符号是否存在
      auto result = std::make_shared<SymbolItem_function_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_function_call_c>(node);
      result->name = real_node->id->toId().value;
      // 检查符号定义
      auto exist_id = SymbolItem_c::toFunction(checkIdExist(result));
      if (nullptr == exist_id) {
        return false;
      }
      // 关联函数声明
      exist_id->refs.push_back(real_node);
      real_node->symbol = exist_id;
      // 读取参数节点，检查函数参数匹配
      if (false == analyseChildren(node) || false == exist_id->checkFunArgs(real_node->children)) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TFunctionDefine: {
      auto result = std::make_shared<SymbolItem_function_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_function_define_c>(node);
      result->name = real_node->id->toId().value;
      result->type = real_node;
      // 添加函数符号定义
      // 当前函数符号所在的范围
      if (false == currentAddSymbol(result)) {
        return false;
      }
      // 压入新符号范围
      symbolTablePush(real_node.get());
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
      // 检查 body 和 返回值是否匹配
      if (false == SyntaxNode_value_define_c::compare(real_node->returnType(),
                                                      real_node->body->returnType())) {
        return false;
      }
      // 关联符号
      real_node->symbol = result;
    } break;
    case SyntaxNodeType_e::TCtrlIfBranch: {
      auto real_node = HicUtil_c::toType<SyntaxNode_if_branch_c>(node);
      symbolTablePush(real_node->if_body.get());
      // 读取 expr || body
      if (false == analyseNodeList(real_node->if_expr, real_node->if_body)) {
        return false;
      }
      // 检查 if_expr 返回 bool 类型
      if (false == real_node->if_expr->isReturnBool()) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TCtrlIf: {
      auto real_node = HicUtil_c::toType<SyntaxNode_if_c>(node);
      // 检查 branch
      for (auto item : real_node->branchs) {
        if (false == analyseNode(item)) {
          return false;
        }
      }
    } break;
    case SyntaxNodeType_e::TCtrlWhile: {
      auto real_node = HicUtil_c::toType<SyntaxNode_while_c>(node);
      symbolTablePush(real_node->body.get());
      // 检查
      if (false == analyseNode(real_node->loop_expr) || false == analyseNode(real_node->body)) {
        return false;
      }
      // 检查循环条件需要为 bool 类型
      if (false == real_node->loop_expr->isReturnBool()) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TCtrlFor: {
      // TODO: 将 for 的 [start_expr] 和 [body] 划分为两个符号范围？
      auto real_node = HicUtil_c::toType<SyntaxNode_for_c>(node);
      symbolTablePush(real_node.get());
      // 检查
      if (false == tryAnalyseNodeList(real_node->start_expr, real_node->loop_expr,
                                      real_node->loop_end_expr) ||
          false == analyseNode(real_node->body)) {
        return false;
      }
      // 检查循环条件需要为 bool 类型
      if (false == real_node->loop_expr->isReturnBool()) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TCtrlReturn: {
      auto real_node = HicUtil_c::toType<SyntaxNode_ctrl_return_c>(node);
      // 允许:
      // - nullptr; return;
      // - data;    return data;
      if (false == tryAnalyseNode(real_node->data)) {
        return false;
      }
      real_node->set_return_type(real_node->data->returnType());
    } break;
    case SyntaxNodeType_e::TEnumDefine: {
      auto result = std::make_shared<SymbolItem_enum_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_enum_define_c>(node);
      result->name = real_node->id->id;
      result->type = real_node;
      // 检查声明位置为全局区
      if (symbolTableStack.size() != 1) {
        SemLog(Terror, "Enum 应当声明在全局区");
        return false;
      }
      // 添加枚举名符号
      if (false == currentAddSymbol(result)) {
        return false;
      }
      symbolTablePush(real_node.get());
      // TODO: 检查重命名，添加枚举变量符号
      if (false == analyseChildren(real_node)) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TClassDefine: {
      symbolTablePush();
      // TODO: class
    } break;
    case SyntaxNodeType_e::TOperator: {
      auto real_node = HicUtil_c::toType<SyntaxNode_operator_c>(node);
      switch (real_node->oper) {
      case WordEnumOperator_e::TUndefined:
      case WordEnumOperator_e::TLevel1:
      case WordEnumOperator_e::TLevel2:
      case WordEnumOperator_e::TEND: {
        // 不需要操作
        Assert_d(real_node->children.empty() == true, "标记类操作符不应包含操作数 {} ({})",
                 WordEnumOperator_c::toName(real_node->oper), real_node->children.size());
      } break;
      case WordEnumOperator_e::TNone:
        break;
      case WordEnumOperator_e::TNot:
      case WordEnumOperator_e::TShift:
      case WordEnumOperator_e::TEndAddAdd:
      case WordEnumOperator_e::TEndSubSub:
      case WordEnumOperator_e::TStartAddAdd:
      case WordEnumOperator_e::TStartSubSub: {
        // 单一操作数 int 型
        Assert_d(real_node->children.size() == 1, "{} 操作符预期需要 1 个操作数，但包含了 {} 个",
                 WordEnumOperator_c::toName(real_node->oper), real_node->children.size());
        if (false == analyseChildren(real_node, 1)) {
          return false;
        }
        // TODO: 断言 int
        real_node->set_return_type(real_node->children.front()->returnType());
      } break;
      default: {
        // 两个操作数
        Assert_d(real_node->children.size() == 2, "{} 操作符预期需要 2 个操作数，但包含了 {} 个",
                 WordEnumOperator_c::toName(real_node->oper), real_node->children.size());
        if (false == analyseChildren(real_node, 2)) {
          return false;
        }
      } break;
      }
    } break;
    }

    // 恢复符号表层级
    if (symbolTableDeep != symbolTableStack.size()) {
      symbolTablePop();
    }
    return true;
  }

  bool analyseChildren(std::shared_ptr<SyntaxNode_c> node, int size = -1) {
    if (size >= 0 && node->children.size() != size) {
      SemLog(Terror, "{} 预期需要 2 个操作数，但包含了 {} 个", node->name(), node->children.size());
      return false;
    }
    // 读取子节点
    for (auto& item : node->children) {
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
        auto word_node = HicUtil_c::toType<WordItem_c>(item);
        switch (word_node->token) {
          // TODO: 关联符号，确定类型
        }
      } break;
      case ListNodeType_e::Syntactic: {
        auto real_node = analyseNode(HicUtil_c::toType<SyntaxNode_c>(item));
        if (false == real_node) {
          return false;
        }
      } break;
      case ListNodeType_e::Symbol:
        return false;
      }
    }
    return true;
  }

  bool analyse() {
    // 语法分析
    auto syntactic_result = syntacticAnalysis.analyse();
    if (false == syntactic_result) {
      return false;
    }
    UtilLog(Tinfo, "语义分析：");
    // 语义分析
    auto result = analyseNode(syntacticAnalysis.root);
    if (result) {
      // success
      Assert_d(symbolTableStack.size() == 1, "解析完成时符号表层级不是 1（{}）",
               symbolTableStack.size());
    }
    return result;
  }

  bool currentAddSymbol(std::shared_ptr<SymbolItem_c> item) {
    if (false == checkIdDefine(item)) {
      return false;
    }
    currentSymbolTable()->insert(std::pair{item->name, item});
  }

  std::shared_ptr<SymbolTable>& globalSymbolTable() {
    Assert_d(symbolTableStack.empty() == false);
    return symbolTableStack.front();
  }

  std::shared_ptr<SymbolTable>& currentSymbolTable() {
    Assert_d(symbolTableStack.empty() == false);
    SemLog(Tdebug, "SymbolTable.size(): {}", symbolTableStack.size());
    return symbolTableStack.back();
  }

  std::shared_ptr<SymbolTable>& symbolTablePush(SyntaxNode_group_c* group) {
    auto& table = symbolTablePush();
    if (nullptr != group) {
      group->symbolTable = table;
    }
    return table;
  }

  std::shared_ptr<SymbolTable>& symbolTablePush() {
    return symbolTableStack.emplace_back(std::make_shared<SymbolTable>());
  }

  void symbolTablePop() { symbolTableStack.pop_back(); }

  // 在 [symbolTable] 中查找符号，且是从最近/最小的作用域开始查找
  std::shared_ptr<SymbolItem_c> symbolTableFind(const std::string& key) {
    if (false == symbolTableStack.empty()) {
      for (auto it = symbolTableStack.end() - 1;; --it) {
        if ((*it)->contains(key)) {
          return (**it)[key];
        }
        if (it == symbolTableStack.begin()) {
          break;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SymbolItem_c> globalSymbolTableFind(const std::string& key) {
    auto& curr = globalSymbolTable();
    if (curr->contains(key)) {
      return (*curr)[key];
    }
    return nullptr;
  }

  std::shared_ptr<SymbolItem_c> currentSymbolTableFind(const std::string& key) {
    auto& curr = currentSymbolTable();
    if (curr->contains(key)) {
      return (*curr)[key];
    }
    return nullptr;
  }

  void debugPrintSymbolTable() {
    int i = 0;
    for (const auto& table : symbolTableStack) {
      i++;
      std::cout << i << std::endl;
      for (const auto& it : *table) {
        std::cout << "- " << it.second->name << " : " << std::endl;
        it.second->debugPrint();
      }
    }
  }

  std::shared_ptr<SyntaxNode_c> tree() { return syntacticAnalysis.root; }

  // 作用域符号表
  std::vector<std::shared_ptr<SymbolTable>> symbolTableStack{};
  SyntacticAnalysis_c syntacticAnalysis{};
};

#undef SemLog