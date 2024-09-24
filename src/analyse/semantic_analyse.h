#pragma once

// 语义分析

#include "syntactic_analyse.h"

// 符号管理
class SymbolManager_c {
public:
  SymbolManager_c() {}

  void init() { stack.clear(); }

  bool checkIdDefine(std::shared_ptr<SymbolItem_c> symbol) {
    Assert_d(nullptr != symbol, "符号不应为 nullptr");
    Assert_d(false == symbol->name.empty(), "符号名称不应为空");
    auto& table = currentTable();
    if (table->find(symbol->name) != table->end()) {
      // 重定义
      UtilLog(Terror, "重定义符号: {}", symbol->name);
      return false;
    }
    if (false == symbol->hasType()) {
      // 缺少类型
      UtilLog(Terror, "符号声明缺少类型: {}", symbol->name);
      return false;
    }
    return true;
  }

  std::shared_ptr<SymbolItem_c> checkIdExist(const std::shared_ptr<SymbolItem_c>& symbol) {
    Assert_d(nullptr != symbol, "符号名称不应为空");
    if (nullptr == symbol) {
      return nullptr;
    }
    return checkIdExist(symbol->name);
  }
  std::shared_ptr<SymbolItem_c> checkIdExist(const std::string& name) {
    Assert_d(false == name.empty(), "符号名称不应为空");
    auto result = find(name);
    if (nullptr == result) {
      UtilLog(Terror, "未定义符号: {}", name);
    }
    return result;
  }

  bool currentIsGlobal() { return (stack.size() == 1); }

  bool currentAddSymbol(std::shared_ptr<SymbolItem_c> item) {
    if (false == checkIdDefine(item)) {
      return false;
    }
    currentTable()->insert(std::pair{item->name, item});
    return true;
  }

  std::shared_ptr<SymbolTable>& globalTable() {
    Assert_d(stack.empty() == false);
    return stack.front();
  }

  std::shared_ptr<SymbolTable>& currentTable() {
    Assert_d(stack.empty() == false);
    return stack.back();
  }

  std::shared_ptr<SymbolTable>& push(SyntaxNode_group_c* group) {
    if (nullptr != group) {
      if (nullptr != group->symbolTable) {
        stack.push_back(group->symbolTable);
        return group->symbolTable;
      }
    }
    auto& table = push();
    if (nullptr != group) {
      // 传入 [group] 且它没有绑定符号表
      group->symbolTable = table;
    }
    return table;
  }

  std::shared_ptr<SymbolTable>& push() {
    return stack.emplace_back(std::make_shared<SymbolTable>());
  }

  void pop() { stack.pop_back(); }

  // 在 [symbolTable] 中查找符号，且是从最近/最小的作用域开始查找
  std::shared_ptr<SymbolItem_c> find(const std::string& key) {
    if (false == stack.empty()) {
      for (auto it = stack.end() - 1;; --it) {
        if ((*it)->contains(key)) {
          return (**it)[key];
        }
        if (it == stack.begin()) {
          break;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SymbolItem_c> findValue(const std::string& key) {
    return SymbolItem_c::toValue(find(key));
  }
  std::shared_ptr<SymbolItem_function_c> findFunction(const std::string& key) {
    return SymbolItem_c::toFunction(find(key));
  }

  std::shared_ptr<SymbolItem_c> globalFind(const std::string& key) {
    auto& curr = globalTable();
    if (curr->contains(key)) {
      return (*curr)[key];
    }
    return nullptr;
  }

  std::shared_ptr<SymbolItem_c> currentFind(const std::string& key) {
    auto& curr = currentTable();
    if (curr->contains(key)) {
      return (*curr)[key];
    }
    return nullptr;
  }

  void debugPrint() {
    int i = 0;
    for (const auto& table : stack) {
      i++;
      std::cout << i << std::endl;
      for (const auto& it : *table) {
        std::cout << "- " << it.second->name << " : " << std::endl;
        it.second->debugPrint();
      }
    }
  }

  // 作用域符号表
  std::vector<std::shared_ptr<SymbolTable>> stack{};
};

class SemanticAnalyse_c {
public:
  inline static bool enableLog_analyseNode = false;

  bool init(std::string_view in_code) {
    symbolManager = std::make_shared<SymbolManager_c>();
    symbolManager->init();
    return syntacticAnalysis.init(in_code);
  }

  template <typename... _ARGS> bool analyseNodeList(std::shared_ptr<_ARGS>... args) {
    return (analyseNode(args) && ...);
  }

  template <typename... _ARGS> bool tryAnalyseNodeList(std::shared_ptr<_ARGS>... args) {
    return (tryAnalyseNode(args) && ...);
  }

  bool checkChildrenType_int(std::shared_ptr<SyntaxNode_c> node) {
    int index = node->children.size();
    for (const auto& item : node->children) {
      auto returnType = item->returnType();
      if (nullptr == returnType) {
        UtilLog(Terror, "操作符 {} 的操作数 [{}]({}) 类型不应为空", node->name(), index,
                item->name());
        return false;
      } else if (false == returnType->isIntValue()) {
        UtilLog(Terror, "操作符 {} 的操作数 [{}]({}) 类型不是 int:", node->name(), index,
                item->name());
        returnType->printInfo();
        return false;
      }
      --index;
    }
    return true;
  }

  bool checkChildrenType_bool(std::shared_ptr<SyntaxNode_c> node) {
    int index = node->children.size();
    for (const auto& item : node->children) {
      auto returnType = item->returnType();
      if (nullptr == returnType) {
        UtilLog(Terror, "操作符 {} 的操作数 [{}]({}) 类型不应为空", node->name(), index,
                item->name());
        return false;
      } else if (false == returnType->isBoolValue()) {
        UtilLog(Terror, "操作符 {} 的操作数 [{}]({}) 类型不是 bool:", node->name(), index,
                item->name());
        returnType->printInfo();
        return false;
      }
      --index;
    }
    return true;
  }

  bool analyseNode_operator(std::shared_ptr<SyntaxNode_c> node) {
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
    case WordEnumOperator_e::TNone: {
      if (false == analyseChildren(real_node)) {
        return false;
      }
      if (real_node->children.size() == 1) {
        real_node->set_return_type(real_node->children.front()->returnType());
      } else {
        Assert_d(real_node->children.size() == 1, "操作符包含过多节点 {} ",
                 real_node->children.size());
        return false;
      }
    } break;
    case WordEnumOperator_e::TNot: {
      // !
      // 单一操作数 bool 型
      if (false == analyseChildren(real_node, 1)) {
        return false;
      }
      // 检查 bool
      if (false == checkChildrenType_bool(real_node)) {
        return false;
      }
      real_node->make_return_type(WordEnumType_e::Tbool, TypeLimit_e::Final);
    } break;
    case WordEnumOperator_e::TShift: {
      // 单一操作数 int 型
      if (false == analyseChildren(real_node, 1)) {
        return false;
      }
      // 检查 int
      if (false == checkChildrenType_int(real_node)) {
        return false;
      }
      real_node->make_return_type(WordEnumType_e::Tint, TypeLimit_e::Final);
    } break;
    case WordEnumOperator_e::TEndAddAdd:
    case WordEnumOperator_e::TEndSubSub:
    case WordEnumOperator_e::TStartAddAdd:
    case WordEnumOperator_e::TStartSubSub: {
      // 单一操作数 int 型
      if (false == analyseChildren(real_node, 1)) {
        return false;
      }
      // 检查 int
      if (false == checkChildrenType_int(real_node)) {
        return false;
      }
      auto type = real_node->children.front()->returnType();
      real_node->set_return_type(type);
    } break;
    case WordEnumOperator_e::TBitAnd: { // & 取址 / 按位与
      // 1 或 2 个操作数
      if (false == analyseChildren(real_node, 1, 2)) {
        return false;
      }
      if (real_node->children.size() == 1) {
        // 取址
        auto crude_type = real_node->children.front()->returnType();
        if (nullptr == crude_type) {
          UtilLog(Terror, "操作符 {} 的操作数类型不应为空", real_node->name());
          return false;
        }
        if (crude_type->isConstexpr()) {
          UtilLog(Terror, "不允许对字面量取址: {}", real_node->name());
          return false;
        }
        auto type = std::make_shared<Type_c>(*crude_type); // 复制类型
        type->isReferer = false;                           // 加一级指针
        type->pointer++;
        real_node->set_return_type(type);
      } else if (real_node->children.size() == 2) {
        // 按位与
        // 检查 int,int
        if (false == checkChildrenType_int(real_node)) {
          return false;
        }
        real_node->make_return_type(WordEnumType_e::Tint, TypeLimit_e::Final);
      } else {
        return false;
      }
    } break;
    case WordEnumOperator_e::TMulti: { // * 读址 / 乘法
      // 1 或 2 个操作数
      if (false == analyseChildren(real_node, 1, 2)) {
        return false;
      }
      if (real_node->children.size() == 1) {
        // 读址
        auto crude_type = real_node->children.front()->returnType();
        if (nullptr == crude_type) {
          UtilLog(Terror, "操作符 {} 的操作数类型不应为空", real_node->name());
          return false;
        }
        auto type = std::make_shared<Type_c>(*crude_type);
        if (type->pointer <= 0) {
          UtilLog(Terror, "操作符 {} 的操作数需要指针类型", real_node->name());
          return false;
        }
        type->isReferer = false;
        type->pointer--;
        real_node->set_return_type(type);
      } else if (real_node->children.size() == 2) {
        // 乘法
        // 检查 int,int
        if (false == checkChildrenType_int(real_node)) {
          return false;
        }
        real_node->make_return_type(WordEnumType_e::Tint, TypeLimit_e::Final);
      } else {
        return false;
      }
    } break;
    case WordEnumOperator_e::TBitLeftMove:
    case WordEnumOperator_e::TBitRightMove: {
      // 两个操作数都需要是 int
      if (false == analyseChildren(real_node, 2)) {
        return false;
      }
      // 检查 int, int
      if (false == checkChildrenType_int(real_node)) {
        return false;
      }
      auto left = real_node->children.back()->returnType();
      if (false == left->canModify()) {
        UtilLog(Terror, "操作符 {} 的操作数[{}]({}) 不可修改", node->name(),
                real_node->children.back()->name(), 0);
        return false;
      }
      real_node->set_return_type(left);
    } break;
    case WordEnumOperator_e::TAnd:
    case WordEnumOperator_e::TOr: {
      // 两个操作数
      if (false == analyseChildren(real_node, 2)) {
        return false;
      }
      if (false == checkChildrenType_bool(real_node)) {
        return false;
      }
      // 返回 bool
      real_node->make_return_type(WordEnumType_e::Tbool, TypeLimit_e::Final);
    } break;
    case WordEnumOperator_e::TEqual:
    case WordEnumOperator_e::TNotEqual: {
      // 两个操作数
      if (false == analyseChildren(real_node, 2)) {
        return false;
      }
      auto first = real_node->children.front()->returnType();
      auto second = real_node->children.back()->returnType();
      if (false == Type_c::compare(first, second)) {
        UtilLog(Terror, "变量类型不相符：");
        UtilLog(Tinfo, "fisrt:");
        real_node->children.front()->printInfo();
        UtilLog(Tinfo, "second:");
        real_node->children.back()->printInfo();
        return false;
      }
      // 返回 bool
      real_node->make_return_type(WordEnumType_e::Tbool, TypeLimit_e::Final);
    } break;
    case WordEnumOperator_e::TSetBitAnd:
    case WordEnumOperator_e::TSetBitOr: {
      // 两个操作数都需要是 int，且 左操作数 应可修改
      if (false == analyseChildren(real_node, 2)) {
        return false;
      }
      // 检查 int, int
      if (false == checkChildrenType_int(real_node)) {
        return false;
      }
      auto left = real_node->children.back()->returnType();
      if (false == left->canModify()) {
        UtilLog(Terror, "操作符 {} 的操作数[{}]({}) 不可修改", node->name(),
                real_node->children.back()->name(), 0);
        return false;
      }
      real_node->set_return_type(left);
    } break;
    case WordEnumOperator_e::TSet:
    case WordEnumOperator_e::TSetAdd:
    case WordEnumOperator_e::TSetSub:
    case WordEnumOperator_e::TSetMulti:
    case WordEnumOperator_e::TSetDivision:
    case WordEnumOperator_e::TSetNullMerge: {
      // 两个操作数
      if (false == analyseChildren(real_node, 2)) {
        return false;
      }
      auto left = real_node->children.back()->returnType();
      if (false == left->canModify()) {
        UtilLog(Terror, "操作符 {} 的操作数[{}]({}) 不可修改", node->name(),
                real_node->children.back()->name(), 0);
        return false;
      }
      real_node->set_return_type(left);
    } break;
    default: {
      // 两个操作数
      if (false == analyseChildren(real_node, 2)) {
        return false;
      }
      auto left = real_node->children.back()->returnType();
      real_node->set_return_type(left);
    } break;
    }
    if (real_node->returnType() == nullptr) {
      UtilLog(Terror, "操作符 {} 的返回值类型不应为空", real_node->name());
      real_node->debugPrint();
      return false;
    }
    return true;
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
    int symbolTableDeep = symbolManager->stack.size();
    // [1]
    switch (node->syntaxType) {
    case SyntaxNodeType_e::TNormal:
    case SyntaxNodeType_e::TValueDefine: {
      if (false == analyseChildren(node)) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TGroup: {
      // {} 隔离符号范围
      auto real_node = HicUtil_c::toType<SyntaxNode_group_c>(node);
      symbolManager->push(real_node.get());
      if (false == analyseChildren(node)) {
        return false;
      }
      // 和子节点保持相同返回值类型
      for (const auto& item : real_node->children) {
        auto result = item->returnType();
        if (nullptr != result) {
          real_node->set_return_type(result);
          break;
        }
      }
    } break;
    case SyntaxNodeType_e::TValueDefineId: {
      // 变量定义，添加符号表
      auto result = std::make_shared<SymbolItem_value_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_value_define_id_c>(node);
      result->type = real_node->value_define->value_type;
      result->name = real_node->id->id;
      real_node->symbol = result;
      // 添加符号定义
      if (false == symbolManager->currentAddSymbol(result)) {
        node->debugPrint();
        return false;
      }
    } break;
    case SyntaxNodeType_e::TValueDefineInit: {
      auto real_node = HicUtil_c::toType<SyntaxNode_value_define_init_c>(node);
      // 添加符号定义，解析 data
      if (false == analyseNodeList(real_node->define_id, real_node->data)) {
        return false;
      }
      // 检查类型
      if (false ==
          Type_c::compare(real_node->define_id->symbol->type, real_node->data->returnType())) {
        return false;
      }
      // TODO: 如果初始化值是已知的简单值，直接写入，否则运行时执行
      // const 必须初始化编译器已知值
      // result->value = real_node->data->returnType()->value_type;
    } break;
    case SyntaxNodeType_e::TNativeFunctionCall:
    case SyntaxNodeType_e::TUserFunctionCall: {
      // 检查符号是否存在
      auto result = std::make_shared<SymbolItem_function_c>();
      auto real_node = std::shared_ptr<SyntaxNode_function_call_base_c>{};
      switch (node->syntaxType) {
      case SyntaxNodeType_e::TNativeFunctionCall: {
        auto native_node = HicUtil_c::toType<SyntaxNode_native_call_c>(node);
        result->name = native_node->id->name();
        real_node = native_node;
      } break;
      case SyntaxNodeType_e::TUserFunctionCall: {
        auto fun_node = HicUtil_c::toType<SyntaxNode_function_call_c>(node);
        result->name = fun_node->id->name();
        real_node = fun_node;
      } break;
      default:
        break;
      }
      // 检查符号定义
      auto exist_id = SymbolItem_c::toFunction(symbolManager->checkIdExist(result));
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
    case SyntaxNodeType_e::TNativeFunctionDefine: {
      auto result = std::make_shared<SymbolItem_function_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_function_define_native_c>(node);
      result->name = real_node->id->name();
      result->type = real_node;
      // 添加函数符号定义
      // 当前函数符号所在的范围
      if (false == symbolManager->currentAddSymbol(result)) {
        return false;
      }
      // 压入新符号范围
      symbolManager->push(real_node.get());
      // 读取 args
      for (const auto& item : real_node->args) {
        if (false == analyseNode(item)) {
          return false;
        }
      }
      // 关联符号
      real_node->symbol = result;
    } break;
    case SyntaxNodeType_e::TUserFunctionDefine: {
      auto result = std::make_shared<SymbolItem_function_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_function_define_user_c>(node);
      result->name = real_node->id->name();
      result->type = real_node;
      // 添加函数符号定义
      // 当前函数符号所在的范围
      if (false == symbolManager->currentAddSymbol(result)) {
        return false;
      }
      // 压入新符号范围
      symbolManager->push(real_node.get());
      // 读取 args
      for (const auto& item : real_node->args) {
        if (false == analyseNode(item)) {
          return false;
        }
      }
      // 读取body
      if (false == analyseNode(real_node->body)) {
        return false;
      }
      // 检查 body 和 返回值是否匹配
      if (false == Type_c::compare(real_node->returnType(), real_node->body->returnType())) {
        return false;
      }
      // 关联符号
      real_node->symbol = result;
    } break;
    case SyntaxNodeType_e::TCtrlIfBranch: {
      auto real_node = HicUtil_c::toType<SyntaxNode_if_branch_c>(node);
      symbolManager->push(real_node->if_body.get());
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
      for (const auto& item : real_node->branchs) {
        if (false == analyseNode(item)) {
          return false;
        }
      }
    } break;
    case SyntaxNodeType_e::TCtrlWhile: {
      auto real_node = HicUtil_c::toType<SyntaxNode_while_c>(node);
      symbolManager->push(real_node->body.get());
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
      symbolManager->push(real_node.get());
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
    } break;
    case SyntaxNodeType_e::TEnumDefine: {
      auto result = std::make_shared<SymbolItem_enum_c>();
      auto real_node = HicUtil_c::toType<SyntaxNode_enum_define_c>(node);
      result->name = real_node->id->id;
      result->type = real_node;
      // 检查声明位置为全局区
      if (symbolManager->stack.size() != 1) {
        UtilLog(Terror, "Enum 应当声明在全局区");
        return false;
      }
      // 添加枚举名符号
      if (false == symbolManager->currentAddSymbol(result)) {
        return false;
      }
      symbolManager->push(real_node.get());
      // 检查重命名，添加枚举变量符号
      if (false == analyseChildren(real_node)) {
        return false;
      }
    } break;
    case SyntaxNodeType_e::TClassDefine: {
      symbolManager->push();
      // TODO: class
    } break;
    case SyntaxNodeType_e::TOperator: {
      if (false == analyseNode_operator(node)) {
        return false;
      }
    } break;
    }

    // 恢复符号表层级
    if (symbolTableDeep != symbolManager->stack.size()) {
      symbolManager->pop();
    }
    return true;
  }

  bool tryAnalyseNode(std::shared_ptr<SyntaxNode_c> node) {
    if (nullptr == node) {
      return true;
    }
    return analyseNode(node);
  }

  template <typename... _Args>
  bool analyseChildren(std::shared_ptr<SyntaxNode_c> node, int size = -1, _Args... args) {
    Assert_d(nullptr != node, "node 不应为空");
    if (nullptr == node) {
      return true;
    }
    if (size >= 0 && node->children.size() != size && ((node->children.size() != args) && ...)) {
      UtilLog(Terror, "{} 包含了 {} 个参数，但预期需要:", node->name(), node->children.size());
      std::cout << size;
      ((std::cout << " / ", std::cout << args), ...);
      std::cout << std::endl;
      return false;
    }
    // 读取子节点
    int index = 0;
    for (const auto& item : node->children) {
      index++;
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
        // 仅 operator 需要
        if (SyntaxNodeType_e::TOperator != node->syntaxType) {
          continue;
        }
        auto word_node = HicUtil_c::toType<WordItem_c>(item);
        switch (word_node->token) {
        // TODO: 分配常量区
        case WordEnumToken_e::Tid: {
          auto& real_node = word_node->toId();
          // 查找符号
          auto exist_id = symbolManager->checkIdExist(real_node.id);
          if (nullptr == exist_id) {
            return false;
          }
          // 不会是 function
          switch (exist_id->symbolType) {
          case SymbolType_e::TValue: {
            real_node.return_type = SymbolItem_c::toValue(exist_id)->type;
          } break;
          case SymbolType_e::TEnum:
          case SymbolType_e::TClass: {
            // TODO: 关联其他值类型符号
          } break;
          case SymbolType_e::TFunction:
          default: {
            UtilLog(Terror, "操作符 ID叶子节点 不能是函数名称：{}",
                    SymbolItem_c::toFunction(exist_id)->name);
            return false;
          } break;
          }
        } break;
        case WordEnumToken_e::Ttype: {
          // TODO: 循环引用，内存泄漏
          // auto& real_node = word_node->toType();
          // real_node->make_return_type(real_node->value);
          // real_node.return_type->value_type = word_node;
        } break;
        case WordEnumToken_e::Tvalue: {
          auto& real_node = word_node->toValue();
          // TODO: 循环引用，内存泄漏
          switch (real_node.value) {
          case WordEnumValue_e::Tnullptr: {
            real_node.make_return_type(WordEnumType_e::Tvoid);
            real_node.return_type->limit = TypeLimit_e::Constexpr;
            real_node.return_type->pointer = Type_c::anyPointer;
            real_node.return_type->word = word_node;
          } break;
          case WordEnumValue_e::Ttrue:
          case WordEnumValue_e::Tfalse: {
            real_node.make_return_type(WordEnumType_e::Tbool);
            real_node.return_type->limit = TypeLimit_e::Constexpr;
            real_node.return_type->word = word_node;
          } break;
          default:
            break;
          }
        } break;
        case WordEnumToken_e::Tkeyword: {
        } break;
        case WordEnumToken_e::TnativeCall: {
          auto& real_node = word_node->toNativeCall();
          // 返回对应函数返回值类型
          // 寻找对应的函数类型声明
          auto exist_id = symbolManager->find(real_node.name());
          if (nullptr == exist_id) {
            UtilLog(Terror, "内置函数调用了未声明的函数类型：{}", real_node.name());
            return false;
          }
          real_node.return_type = exist_id->returnType();
        } break;
        case WordEnumToken_e::Tnumber: {
          auto& real_node = word_node->toNumber();
          real_node.make_return_type(WordEnumType_e::Tint, TypeLimit_e::Constexpr, word_node);
        } break;
        case WordEnumToken_e::Tstring: {
          auto& real_node = word_node->toString();
          real_node.make_return_type(WordEnumType_e::TString, TypeLimit_e::Constexpr, word_node);
        } break;
        }
      } break;
      case ListNodeType_e::Syntactic: {
        auto real_node = analyseNode(HicUtil_c::toType<SyntaxNode_c>(item));
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
      Assert_d(symbolManager->stack.size() == 0, "解析完成时符号表层级不是 0（{}）",
               symbolManager->stack.size());
    }
    return result;
  }

  std::shared_ptr<SyntaxNode_group_c> tree() { return syntacticAnalysis.root; }

  SyntacticAnalysis_c syntacticAnalysis{};
  std::shared_ptr<SymbolManager_c> symbolManager{};
};
