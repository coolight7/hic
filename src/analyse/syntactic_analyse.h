#pragma once

// 语法分析

#include <functional>
#include <list>
#include <memory>
#include <stack>

#include "lexical_analyse.h"
#include "magic/macro.h"
#include "util.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 `,逗号` 给 UtilLog导致展开异常
 */
#define SynLog(level, tip, ...)                                                                    \
  UtilLog(level,                                                                                   \
          "[" + WordEnumToken_c::toName(lexicalAnalyse.currentToken()->token) + "] " +             \
              lexicalAnalyse.currentToken()->name(),                                               \
          lexicalAnalyse.current_line, tip, ##__VA_ARGS__)

#define _GEN_WORD(name)                                                                            \
  if (nullptr == name##_ptr) {                                                                     \
    name##_ptr = lexicalAnalyse.analyse();                                                         \
    if (nullptr == name##_ptr) {                                                                   \
      return nullptr;                                                                              \
    }                                                                                              \
  }                                                                                                \
  auto& name = *name##_ptr.get();

#define _GEN_WORD_DEF(name)                                                                        \
  std::shared_ptr<WordItem_c> name##_ptr;                                                          \
  _GEN_WORD(name)

#define _GENERATE_FUN_ITEM_d(...)                                                                  \
  _NameTagConcat_d(_GENERATE_FUN_ITEM, _MacroArgToTag_d(__VA_ARGS__))(__VA_ARGS__)
#define _GENERATE_FUN_ITEM() _GENERATE_FUN_ITEM_d
#define _GENERATE_FUN_ITEM1(fun)                                                                   \
  std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<WordItem_c>)>(                       \
      std::bind(&SyntacticAnalysis_c::fun, this, std::placeholders::_1))
#define _GENERATE_FUN_ITEMN(fun, ...)                                                              \
  _GENERATE_FUN_ITEM1(fun), _MacroDefer_d(_GENERATE_FUN_ITEM)()(__VA_ARGS__)

/**
 * ## 回溯分析
 * - 见 [reback_funs]
 */
#define _REBACK_d(word_ptr, tempIndex, ...)                                                        \
  reback_funs(word_ptr, tempIndex, _MoreExpand_d(_GENERATE_FUN_ITEM_d(__VA_ARGS__)))

#define _GEN_VALUE(type, name)                                                                     \
  bool set_##name(std::shared_ptr<type> in_ptr) {                                                  \
    if (nullptr == in_ptr) {                                                                       \
      return false;                                                                                \
    }                                                                                              \
    name = in_ptr;                                                                                 \
    return true;                                                                                   \
  }                                                                                                \
  std::shared_ptr<type> name;

#define _PRINT_WORD_PREFIX(isEnd)                                                                  \
  {                                                                                                \
    if (tab > 0) {                                                                                 \
      size_t prefixTab = 0;                                                                        \
      if (nullptr != onOutPrefix) {                                                                \
        prefixTab = onOutPrefix();                                                                 \
      }                                                                                            \
      for (int i = tab - prefixTab - 1; i-- > 0;) {                                                \
        std::cout << "   ";                                                                        \
      }                                                                                            \
    }                                                                                              \
    if (isEnd) {                                                                                   \
      std::cout << "  └──";                                                                        \
    } else {                                                                                       \
      std::cout << "  ├──";                                                                        \
    }                                                                                              \
  }

#define _PRINT_NODE_PREFIX(isEnd, name)                                                            \
  {                                                                                                \
    size_t prefixTab = 0;                                                                          \
    if (nullptr != onOutPrefix) {                                                                  \
      prefixTab = onOutPrefix();                                                                   \
    }                                                                                              \
    for (int i = tab - prefixTab - 1; i-- > 0;) {                                                  \
      std::cout << "   ";                                                                          \
    }                                                                                              \
    bool isEndValue = isEnd;                                                                       \
    if (isEndValue) {                                                                              \
      std::cout << "  └──┐";                                                                       \
    } else {                                                                                       \
      std::cout << "  ├──┐";                                                                       \
    }                                                                                              \
    std::cout << std::endl;                                                                        \
    SyntaxNode_c* name##_ptr = (SyntaxNode_c*)name.get();                                          \
    name##_ptr->debugPrint(tab + 1, [&onOutPrefix, isEndValue]() -> size_t {                       \
      size_t size = 0;                                                                             \
      if (nullptr != onOutPrefix) {                                                                \
        size = onOutPrefix();                                                                      \
      }                                                                                            \
      if (false == isEndValue) {                                                                   \
        std::cout << "  │";                                                                        \
      } else {                                                                                     \
        std::cout << "   ";                                                                        \
      }                                                                                            \
      return size + 1;                                                                             \
    });                                                                                            \
  }

enum SyntaxNodeType_e {
  Normal, // 分组节点，自身无特殊意义
  Group,  // {}，隔离符号范围

  ValueDefine,
  ValueDefineId,
  ValueDefineInit,

  FunctionCall,
  Operator,
  CtrlReturn,
  CtrlIfBranch,
  CtrlIf,
  CtrlWhile,
  CtrlFor,

  FunctionDefine,
  EnumDefine,
  ClassDefine,
};

enum SyntaxNodeValueClass_e {
  Crude,   // 值类型
  Pointer, // 指针
  Referer, // 引用
};

// 语法树节点
class SyntaxNode_c : public ListNode_c {
public:
  template <typename... _Args>
  inline static std::shared_ptr<SyntaxNode_c> make_node(std::shared_ptr<_Args>... args) {
    auto re_ptr = std::make_shared<SyntaxNode_c>();
    re_ptr->children.push_back(args...);
    return re_ptr;
  }

  SyntaxNode_c() : ListNode_c(ListNodeType_e::Syntactic), syntaxType(SyntaxNodeType_e::Normal) {}
  SyntaxNode_c(SyntaxNodeType_e type) : ListNode_c(ListNodeType_e::Syntactic), syntaxType(type) {}

  const std::string& name() const override { return HicUtil_c::emptyString; }

  bool add(std::shared_ptr<ListNode_c> ptr) {
    if (nullptr != ptr) {
      children.push_back(ptr);
      return true;
    }
    return false;
  }

  bool isEmpty() { return children.empty(); }

  void printInfo() const override { debugPrint(); }

  virtual void debugPrint(const size_t tab = 1,
                          std::function<size_t()> onOutPrefix = nullptr) const {
    int index = 0;
    for (const auto& item : children) {
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
        // word
        _PRINT_WORD_PREFIX(children.size() == index + 1);
        item->printInfo();
      } break;
      case ListNodeType_e::Syntactic: {
        // Syntax
        _PRINT_NODE_PREFIX((index == children.size() - 1), item);
      } break;
      default:
        break;
      }
      index++;
    }
  }

  SyntaxNodeType_e syntaxType;
  std::list<std::shared_ptr<ListNode_c>> children{};
};

class SyntaxNode_group_c : public SyntaxNode_c {
public:
  SyntaxNode_group_c() : SyntaxNode_c(SyntaxNodeType_e::Group) {}
};

class SyntaxNode_value_define_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_c() : SyntaxNode_c(SyntaxNodeType_e::ValueDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    value_type->printInfo();
    _PRINT_WORD_PREFIX(true);
    if (0 == pointer && false == isReferer) {
      std::cout << "[值类型]";
    } else {
      for (int i = pointer; i > 0; --i) {
        std::cout << "*";
      }
      std::cout << " ";
      if (isReferer) {
        std::cout << "&";
      }
    }
    std::cout << std::endl;
  }

  _GEN_VALUE(WordItem_c, value_type);
  bool isReferer = false; // 是否是引用类型
  size_t pointer = 0;     // 指针层数
};

class SyntaxNode_value_define_id_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_id_c() : SyntaxNode_c(SyntaxNodeType_e::ValueDefineId) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, value_define);
    _PRINT_WORD_PREFIX(true);
    id->printInfo();
  }

  _GEN_VALUE(SyntaxNode_value_define_c, value_define);
  _GEN_VALUE(WordItem_default_c, id);
};

class SyntaxNode_value_define_init_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_init_c() : SyntaxNode_c(SyntaxNodeType_e::ValueDefineInit) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, define_id);
    _PRINT_WORD_PREFIX(false);
    std::cout << "operator -- =" << std::endl;
    _PRINT_NODE_PREFIX(true, data);
  }

  _GEN_VALUE(SyntaxNode_value_define_id_c, define_id);
  // =
  _GEN_VALUE(SyntaxNode_c, data);
};

class SyntaxNode_function_call_c : public SyntaxNode_c {
public:
  SyntaxNode_function_call_c() : SyntaxNode_c(SyntaxNodeType_e::FunctionCall) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(children.empty());
    id->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_c, id);
};

class SyntaxNode_operator_c : public SyntaxNode_c {
public:
  SyntaxNode_operator_c(WordEnumOperator_e in_op)
      : SyntaxNode_c(SyntaxNodeType_e::Operator), oper(in_op) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    if (WordEnumOperator_e::TNone != oper) {
      _PRINT_WORD_PREFIX(children.empty());
      std::cout << "operator -- " << WordItem_operator_c::toSign(oper) << std::endl;
    }
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  WordEnumOperator_e oper;
};

class SyntaxNode_ctrl_return_c : public SyntaxNode_c {
public:
  SyntaxNode_ctrl_return_c() : SyntaxNode_c(SyntaxNodeType_e::CtrlReturn) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "return" << std::endl;
    _PRINT_NODE_PREFIX(true, data);
  }

  _GEN_VALUE(SyntaxNode_operator_c, data);
};

class SyntaxNode_if_branch_c : public SyntaxNode_c {
public:
  SyntaxNode_if_branch_c() : SyntaxNode_c(SyntaxNodeType_e::CtrlIfBranch) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, if_expr);
    _PRINT_NODE_PREFIX(true, if_body);
  }
  // if 条件，如果为 nullptr，则无条件，即为 else_body
  _GEN_VALUE(SyntaxNode_operator_c, if_expr);
  _GEN_VALUE(SyntaxNode_group_c, if_body);
};

class SyntaxNode_if_c : public SyntaxNode_c {
public:
  SyntaxNode_if_c() : SyntaxNode_c(SyntaxNodeType_e::CtrlIf) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    int index = 0;
    for (const auto& item : branchs) {
      _PRINT_WORD_PREFIX(false);
      bool isEnd = (index + 1 == branchs.size());
      if (0 == index) {
        std::cout << "if" << std::endl;
      } else if (isEnd) {
        std::cout << "else" << std::endl;
      } else {
        std::cout << "else if" << std::endl;
      }
      _PRINT_NODE_PREFIX(isEnd, item);
      ++index;
    }
  }

  bool addBranch(std::shared_ptr<SyntaxNode_if_branch_c> branch) {
    if (nullptr == branch) {
      return false;
    }
    branchs.push_back(branch);
    return true;
  }

  std::list<std::shared_ptr<SyntaxNode_if_branch_c>> branchs;
};

class SyntaxNode_while_c : public SyntaxNode_c {
public:
  SyntaxNode_while_c() : SyntaxNode_c(SyntaxNodeType_e::CtrlWhile) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "while" << std::endl;
    _PRINT_NODE_PREFIX(false, loop_expr);
    _PRINT_NODE_PREFIX(true, body);
  }

  _GEN_VALUE(SyntaxNode_operator_c, loop_expr);
  _GEN_VALUE(SyntaxNode_group_c, body);
};

class SyntaxNode_for_c : public SyntaxNode_c {
public:
  SyntaxNode_for_c() : SyntaxNode_c(SyntaxNodeType_e::CtrlFor) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "for" << std::endl;
    _PRINT_NODE_PREFIX(false, start_expr);
    _PRINT_NODE_PREFIX(false, loop_expr);
    _PRINT_NODE_PREFIX(false, loop_end_expr);
    _PRINT_NODE_PREFIX(true, body);
  }

  _GEN_VALUE(SyntaxNode_operator_c, start_expr);
  _GEN_VALUE(SyntaxNode_operator_c, loop_expr);
  _GEN_VALUE(SyntaxNode_operator_c, loop_end_expr);
  _GEN_VALUE(SyntaxNode_group_c, body);
};

class SyntaxNode_function_define_c : public SyntaxNode_c {
public:
  SyntaxNode_function_define_c() : SyntaxNode_c(SyntaxNodeType_e::FunctionDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "Function" << std::endl;
    _PRINT_NODE_PREFIX(false, return_type);
    _PRINT_WORD_PREFIX(false);
    id->printInfo();
    for (const auto& item : args) {
      _PRINT_NODE_PREFIX(false, item);
    }
    _PRINT_NODE_PREFIX(true, body);
  }

  bool addArg(std::shared_ptr<SyntaxNode_value_define_id_c> arg) {
    if (nullptr == arg) {
      return false;
    }
    args.push_back(arg);
    return true;
  }

  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
  _GEN_VALUE(WordItem_c, id);
  std::list<std::shared_ptr<SyntaxNode_value_define_id_c>> args;
  _GEN_VALUE(SyntaxNode_group_c, body);
};

class SyntaxNode_enum_define_c : public SyntaxNode_c {
public:
  SyntaxNode_enum_define_c() : SyntaxNode_c(SyntaxNodeType_e::EnumDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "Enum" << std::endl;
    _PRINT_WORD_PREFIX(false);
    id->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_c, id);
};

class SyntacticAnalysis_c {
public:
  inline static bool enableLog_assertToken = false;
  inline static bool enableLog_parseCode = true;

  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

  /**
   * ## 回溯分析
   * - 见 [reback_funs]
   */
  template <typename T>
  std::shared_ptr<SyntaxNode_c>
  reback(std::shared_ptr<T>&& word_ptr, int tempIndex,
         std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<T>)>&& fun) {
    lexicalAnalyse.tokenIndex = tempIndex;
    return fun(word_ptr);
  }
  /**
   * ## 回溯分析
   * - 见 [reback_funs]
   */
  template <typename T, typename... Args>
  std::shared_ptr<SyntaxNode_c>
  reback(std::shared_ptr<T>&& word_ptr, int tempIndex,
         std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<T>)>&& fun,
         std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<Args>)>&&... funlist) {
    auto result =
        reback(std::forward<std::shared_ptr<T>>(word_ptr), tempIndex,
               std::forward<std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<T>)>>(fun));
    if (nullptr != result) {
      return result;
    }
    if constexpr (sizeof...(funlist) > 0) {
      return reback(
          std::forward<std::shared_ptr<T>>(word_ptr), tempIndex,
          std::forward<std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<Args>)>>(
              funlist)...);
    }
    return nullptr;
  }

  /**
   * ## 回溯分析
   * - 按 [funlist] 的函数顺序依次调用尝试解析从 [tempIndex] 开始的 token，如果分析失败，会将
   * [lexicalAnalyse] 回退到 [tempIndex] 后调用下一个函数尝试，直到有函数成功解析，则结束返回
   * 其返回值；若 [funlist] 的函数都不能解析将返回 [nullptr]
   */
  template <typename... Args>
  std::shared_ptr<SyntaxNode_c>
  reback_funs(std::shared_ptr<WordItem_c> word_ptr, int tempIndex,
              std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<Args>)>&&... funlist) {
    return reback(std::move(word_ptr), tempIndex, std::move(funlist)...);
  }

  std::shared_ptr<WordItem_c> assertToken(const WordItem_c& limit,
                                          std::shared_ptr<WordItem_c> word_ptr = nullptr,
                                          bool startWith = false) {
    _GEN_WORD(word);
    if (limit.token == word.token) {
      if (WordEnumToken_e::Toperator == word.token) {
        Assert_d(false == startWith, "操作符不应使用参数 [startWith]");
        if (limit.toOperator().value == word.toOperator().value) {
          return word_ptr;
        }
      } else if ((startWith && word.name().starts_with(limit.name())) ||
                 (limit.name() == word.name())) {
        return word_ptr;
      }
    }
    if (enableLog_assertToken) {
      SynLog(Twarning, "[assertToken] faild; word: {}, expect: {}", word.toString(),
             limit.toString());
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_type(WordEnumToken_e type,
                                               std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    _GEN_WORD(word);
    if (type == word.token) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_name(const std::string& name,
                                               std::shared_ptr<WordItem_c> word_ptr = nullptr,
                                               bool startWith = false) {
    _GEN_WORD(word);
    if ((startWith && word.name().starts_with(name)) || (name == word.name())) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_sign(WordEnumOperator_e sign,
                                               std::shared_ptr<WordItem_c> word_ptr = nullptr,
                                               bool startWith = false) {
    return assertToken(WordItem_operator_c{sign}, word_ptr, startWith);
  }

  std::shared_ptr<WordItem_c> assertToken_sign(const std::string_view sign,
                                               std::shared_ptr<WordItem_c> word_ptr = nullptr,
                                               bool startWith = false) {
    return assertToken(WordItem_operator_c{WordItem_operator_c::toEnum(sign)}, word_ptr, startWith);
  }

  // 花括号
  std::shared_ptr<WordItem_c> tryParse_brace(
      std::shared_ptr<WordItem_c> word_ptr,
      const std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<WordItem_c>)>& fun) {
    _GEN_WORD(word)
    const auto left_result = assertToken_sign(WordEnumOperator_e::TLeftFlowerGroup, word_ptr);
    if (nullptr != left_result) {
      auto result = tryParse_brace(
          nullptr, [&fun](std::shared_ptr<WordItem_c> next_ptr) { return fun(next_ptr); });
      if (nullptr != result) {
        return assertToken_sign(WordEnumOperator_e::TRightFlowerGroup);
      }
    } else {
      return fun(word_ptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_number_c> parse_constexpr_int(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(WordEnumToken_e::Tnumber, word_ptr);
    if (nullptr != result) {
      return *((std::shared_ptr<WordItem_number_c>*)&result);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_default_c> parse_constexpr_string(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(WordEnumToken_e::Tstring, word_ptr);
    if (nullptr != result) {
      return *((std::shared_ptr<WordItem_default_c>*)&result);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> parse_value_type(std::shared_ptr<WordItem_c> word_ptr) {
    _GEN_WORD(word);
    if (WordEnumToken_e::Ttype == word.token || WordEnumToken_e::Tid == word.token) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_value_define_c>
  parse_value_define(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    // value_type
    auto re_node = std::make_shared<SyntaxNode_value_define_c>();
    if (re_node->set_value_type(parse_value_type(word_ptr))) {
      // 指针或引用
      while (true) {
        _GEN_WORD_DEF(next);
        auto sign_ptr = assertToken_type(WordEnumToken_e::Toperator, next_ptr);
        if (nullptr != sign_ptr) {
          auto& sign = sign_ptr->toOperator();
          if (sign.value == WordEnumOperator_e::TMulti) {
            (re_node->pointer)++;
          } else if (sign.value == WordEnumOperator_e::TBitAnd) {
            re_node->isReferer = true;
            return re_node;
          } else {
            return nullptr;
          }
        } else {
          break;
        }
      }
      // 下一个 [token] 不是符号，回退一个
      lexicalAnalyse.tokenBack();
      return re_node;
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_value_define_id_c>
  parse_value_define_id(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    // value_type
    auto re_node = std::make_shared<SyntaxNode_value_define_id_c>();
    if (re_node->set_value_define(parse_value_define(word_ptr))) {
      // ID
      if (re_node->set_id(
              HicUtil_c::toType<WordItem_default_c>(assertToken_type(WordEnumToken_e::Tid)))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_value_define_init_c>
  parse_value_define_init(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_value_define_init_c>();
    if (re_node->set_define_id(parse_value_define_id(word_ptr))) {
      if (assertToken_sign(WordEnumOperator_e::TSet)) {
        if (re_node->set_data(parse_expr())) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  bool parse_expr_calc(std::stack<std::shared_ptr<ListNode_c>>& dataStack,
                       std::stack<std::shared_ptr<WordItem_operator_c>>& signStack) {
    auto top = signStack.top();
    signStack.pop();
    switch (top->value) {
    case WordEnumOperator_e::TMulti:
    case WordEnumOperator_e::TBitAnd: {
      auto opSizeLimit = dataStack.size() >= 1;
      Assert_d(true == opSizeLimit, "{} 操作数不足1个（{}）",
               WordItem_operator_c::toSign(top->value), dataStack.size());
      if (false == opSizeLimit) {
        return false;
      }
      auto result = std::make_shared<SyntaxNode_operator_c>(top->value);
      result->add(dataStack.top()); // 添加操作数
      dataStack.pop();
      if (false == dataStack.empty()) {
        // - 对于 &
        //    - 若1个操作数，当作 取址 &a
        //    - 若2个操作数，当作 按位与 a & b
        // - 对于 *
        //    - 若1个操作数，当作 读址 *a
        //    - 若2个操作数，当作 乘法 a * b
        result->add(dataStack.top()); // 添加操作数
        dataStack.pop();
      }
      dataStack.push(result); // 添加结果
    } break;
    case WordEnumOperator_e::TEndAddAdd:
    case WordEnumOperator_e::TEndSubSub:
    case WordEnumOperator_e::TNot:
    case WordEnumOperator_e::TShift:
    case WordEnumOperator_e::TStartAddAdd:
    case WordEnumOperator_e::TStartSubSub: {
      auto opSizeLimit = dataStack.size() >= 1;
      Assert_d(true == opSizeLimit, "{} 操作数不足1个（{}）",
               WordItem_operator_c::toSign(top->value), dataStack.size());
      if (false == opSizeLimit) {
        return false;
      }
      auto result = std::make_shared<SyntaxNode_operator_c>(top->value);
      result->add(dataStack.top()); // 添加操作数
      dataStack.pop();
      dataStack.push(result); // 添加结果
    } break;
    case WordEnumOperator_e::TRightSquareGroup: // ] 作为 []
    case WordEnumOperator_e::TDot:
    case WordEnumOperator_e::TNullDot:
    case WordEnumOperator_e::TDivision:
    case WordEnumOperator_e::TPercent:
    case WordEnumOperator_e::TAdd:
    case WordEnumOperator_e::TSub:
    case WordEnumOperator_e::TBitLeftMove:
    case WordEnumOperator_e::TBitRightMove:
    case WordEnumOperator_e::TBitOr:
    case WordEnumOperator_e::TGreaterOrEqual:
    case WordEnumOperator_e::TGreater:
    case WordEnumOperator_e::TLessOrEqual:
    case WordEnumOperator_e::TLess:
    case WordEnumOperator_e::TEqual:
    case WordEnumOperator_e::TNotEqual:
    case WordEnumOperator_e::TAnd:
    case WordEnumOperator_e::TOr:
    case WordEnumOperator_e::TNullMerge:
    case WordEnumOperator_e::TIfElse:
    case WordEnumOperator_e::TSet:
    case WordEnumOperator_e::TSetBitOr:
    case WordEnumOperator_e::TSetBitAnd:
    case WordEnumOperator_e::TSetMulti:
    case WordEnumOperator_e::TSetDivision:
    case WordEnumOperator_e::TSetAdd:
    case WordEnumOperator_e::TSetSub:
    case WordEnumOperator_e::TSetNullMerge: {
      auto opSizeLimit = dataStack.size() >= 2;
      Assert_d(true == opSizeLimit, "{} 操作数不足2个（{}）",
               WordItem_operator_c::toSign(top->value), dataStack.size());
      if (false == opSizeLimit) {
        return false;
      }
      auto result = std::make_shared<SyntaxNode_operator_c>(top->value);
      result->add(dataStack.top()); // 添加操作数 1
      dataStack.pop();
      result->add(dataStack.top()); // 添加操作数 2
      dataStack.pop();
      dataStack.push(result); // 添加结果
    } break;
    }
    return true;
  }

  /**
   * ## 表达式解析
   * - 优先级爬山
   */
  std::shared_ptr<SyntaxNode_operator_c>
  parse_expr(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    std::stack<std::shared_ptr<ListNode_c>> dataStack;
    std::stack<std::shared_ptr<WordItem_operator_c>> signStack;
    while (true) {
      _GEN_WORD(word);
      if (word.token == WordEnumToken_e::Toperator) {
        auto& sign = word.toOperator();
        if (WordEnumOperator_e::TLeftFlowerGroup == sign.value ||
            WordEnumOperator_e::TRightFlowerGroup == sign.value) {
          // 不允许 { }
          return nullptr;
        }
        if (WordEnumOperator_e::TComma == sign.value ||
            WordEnumOperator_e::TSemicolon == sign.value ||
            WordEnumOperator_e::TRightCurvesGroup == sign.value) {
          // , ; ) 退出读取，结算节点
          lexicalAnalyse.tokenBack();
          break;
        }
        if (WordEnumOperator_e::TLeftCurvesGroup == sign.value ||
            WordEnumOperator_e::TLeftSquareGroup == sign.value) {
          // ( [
          // 求子表达式
          auto result = parse_expr();
          if (WordEnumOperator_e::TLeftCurvesGroup == sign.value &&
              assertToken_sign(WordEnumOperator_e::TRightCurvesGroup)) {
            // () 闭合
            dataStack.push(result);
            word_ptr = nullptr;
            continue;
          } else if (WordEnumOperator_e::TLeftSquareGroup == sign.value &&
                     assertToken_sign(WordEnumOperator_e::TRightSquareGroup)) {
            // [] 闭合
            dataStack.push(result);
            // 保留 ] 在 [word_ptr]，expr1[expr2] 需要两个操作数，由下面执行操作
            word_ptr = lexicalAnalyse.currentToken();
            continue;
          } else {
            return nullptr;
          }
        }
        // TODO: 三元运算符 ? :
        while (false == signStack.empty() &&
               WordItem_operator_c::compare(sign.value, signStack.top()->value) > 0) {
          // 新符号优先级更低，将顶部先计算
          auto rebool = parse_expr_calc(dataStack, signStack);
          if (false == rebool) {
            return nullptr;
          }
        }
        signStack.push(*static_cast<std::shared_ptr<WordItem_operator_c>*>((void*)&word_ptr));
      } else {
        if (WordEnumToken_e::Tid == word.token) {
          // 尝试解析函数调用
          auto tempIndex = lexicalAnalyse.tokenIndex;
          auto fun_call = parse_function_call(word_ptr);
          if (fun_call) {
            // 添加节点
            dataStack.push(fun_call);
            word_ptr = nullptr;
            continue;
          } else {
            // 回溯
            lexicalAnalyse.tokenIndex = tempIndex;
          }
        }
        dataStack.push(word_ptr);
      }
      word_ptr = nullptr;
    }
    // 结算节点内容
    while (false == signStack.empty()) {
      auto rebool = parse_expr_calc(dataStack, signStack);
      if (false == rebool) {
        return nullptr;
      }
    }
    if (1 == dataStack.size()) {
      // 套一层类型
      auto re_node = std::make_shared<SyntaxNode_operator_c>(WordEnumOperator_e::TNone);
      re_node->add(dataStack.top());
      return re_node;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c>
  parse_code_ctrl_break(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    return assertToken(WordItem_ctrl_c{WordEnumCtrl_e::Tbreak}, word_ptr);
  }

  std::shared_ptr<WordItem_c>
  parse_code_ctrl_continue(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    return assertToken(WordItem_ctrl_c{WordEnumCtrl_e::Tcontinue}, word_ptr);
  }

  std::shared_ptr<SyntaxNode_ctrl_return_c>
  parse_code_ctrl_return(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    if (assertToken(WordItem_ctrl_c{WordEnumCtrl_e::Treturn}, word_ptr)) {
      auto re_node = std::make_shared<SyntaxNode_ctrl_return_c>();
      if (re_node->set_data(parse_expr())) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_if_c>
  parse_code_ctrl_if(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_if_c>();
    auto first_node = std::make_shared<SyntaxNode_if_branch_c>();
    re_node->addBranch(first_node);
    if (assertToken(WordItem_ctrl_c{WordEnumCtrl_e::Tif}, word_ptr) &&
        assertToken_sign(WordEnumOperator_e::TLeftCurvesGroup) &&
        first_node->set_if_expr(parse_expr()) &&
        assertToken_sign(WordEnumOperator_e::TRightCurvesGroup) &&
        assertToken_sign(WordEnumOperator_e::TLeftFlowerGroup)) {
      // if (expr) { code }
      _GEN_WORD_DEF(sign);
      if (assertToken_sign(WordEnumOperator_e::TRightFlowerGroup, sign_ptr)) {
        // 空代码块 {}
        return re_node;
      }
      if (first_node->set_if_body(parse_code(sign_ptr)) &&
          assertToken_sign(WordEnumOperator_e::TRightFlowerGroup)) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_while_c>
  parse_code_ctrl_while(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_while_c>();
    if (assertToken(WordItem_ctrl_c{WordEnumCtrl_e::Twhile}, word_ptr) &&
        assertToken_sign(WordEnumOperator_e::TLeftCurvesGroup) &&
        re_node->set_loop_expr(parse_expr()) &&
        assertToken_sign(WordEnumOperator_e::TRightCurvesGroup) &&
        assertToken_sign(WordEnumOperator_e::TLeftFlowerGroup)) {
      // while (expr) { code }
      _GEN_WORD_DEF(sign);
      if (assertToken_sign(WordEnumOperator_e::TRightFlowerGroup, sign_ptr)) {
        // 空代码块 {}
        return re_node;
      }
      if (re_node->set_body(parse_code()) &&
          assertToken_sign(WordEnumOperator_e::TRightFlowerGroup)) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_for_c>
  parse_code_ctrl_for(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_for_c>();
    if (assertToken(WordItem_ctrl_c{WordEnumCtrl_e::Tfor}, word_ptr)) {
      if (assertToken_sign(WordEnumOperator_e::TLeftCurvesGroup) &&
          re_node->set_start_expr(parse_expr()) && re_node->set_loop_expr(parse_expr()) &&
          re_node->set_loop_end_expr(parse_expr()) &&
          assertToken_sign(WordEnumOperator_e::TRightCurvesGroup)) {
        // for (expr;expr;expr) { code }
        if (assertToken_sign(WordEnumOperator_e::TLeftFlowerGroup)) {
          _GEN_WORD_DEF(sign);
          if (assertToken_sign(WordEnumOperator_e::TRightFlowerGroup, sign_ptr)) {
            // 空代码块 {}
            return re_node;
          }
          if (re_node->set_body(parse_code()) &&
              assertToken_sign(WordEnumOperator_e::TRightFlowerGroup)) {
            return re_node;
          }
        }
      }
    }
    return nullptr;
  }

  /**
   * 不取外围的大括号 { code }
   */
  std::shared_ptr<SyntaxNode_group_c> parse_code(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_group_c>();
    std::shared_ptr<SyntaxNode_c> result;
    if (enableLog_parseCode) {
      SynLog(Tdebug, "parse_code -----vvv------ ");
    }
    do {
      _GEN_WORD(word);
      if (WordEnumToken_e::Toperator == word.token) {
        auto& sign = word.toOperator();
        if (WordEnumOperator_e::TSemicolon == sign.value) {
          // ;
          word_ptr = nullptr;
          continue;
        } else if (WordEnumOperator_e::TLeftFlowerGroup == sign.value) {
          // { code }
          auto result = parse_code();
          if (re_node->add(result) && assertToken_sign("}")) {
            word_ptr = nullptr;
            continue;
          }
        } else if (WordEnumOperator_e::TRightFlowerGroup == sign.value) {
          // }
          lexicalAnalyse.tokenBack();
          break;
        }
      }
      // word 非终结符
      int tempIndex = lexicalAnalyse.tokenIndex;
      result = _REBACK_d(word_ptr, tempIndex, parse_value_define_init, parse_function_call,
                         parse_code_ctrl_if, parse_code_ctrl_while, parse_code_ctrl_for,
                         parse_code_ctrl_return, parse_expr);
      if (false == re_node->add(result)) {
        // 解析失败
        return nullptr;
      }
      if (enableLog_parseCode) {
        // 打印 code
        if (enableLog_parseCode) {
          SynLog(Tdebug, "  - parse_code/line --vvv-- ");
        }
        result->debugPrint(2);
      }
      word_ptr = nullptr;
    } while (nullptr != result);
    if (enableLog_parseCode) {
      SynLog(Tdebug, "parse_code -----^^^----- ");
    }
    return re_node;
  }

  // {ID_funName} ()
  std::shared_ptr<SyntaxNode_function_call_c>
  parse_function_call(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_function_call_c>();
    if (re_node->set_id(assertToken_type(WordEnumToken_e::Tid, word_ptr))) {
      if (assertToken_sign("(")) {
        std::shared_ptr<WordItem_c> sign_ptr;
        // 参数列表
        while (true) {
          if (false == re_node->add(parse_expr())) {
            break;
          }
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(",", sign_ptr)) {
            break;
          }
          sign_ptr = nullptr;
        }
        if (assertToken_sign(")", sign_ptr)) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_function_define_c>
  parse_function_define(std::shared_ptr<WordItem_c> word_ptr) {
    // {返回值} {函数名} ({参数列表}*) { {code} }
    auto re_node = std::make_shared<SyntaxNode_function_define_c>();
    // 返回值
    if (re_node->set_return_type(parse_value_define(word_ptr))) {
      // 函数名
      if (re_node->set_id(assertToken_type(WordEnumToken_e::Tid))) {
        // 参数列表
        if (assertToken_sign("(")) {
          std::shared_ptr<WordItem_c> sign_ptr;
          _GEN_WORD(sign);
          while (true) {
            if (false == re_node->addArg(parse_value_define_id(sign_ptr))) {
              break;
            }
            sign_ptr = nullptr;
            _GEN_WORD(sign);
            if (nullptr == assertToken_sign(",", sign_ptr)) {
              break;
            }
            sign_ptr = nullptr;
          }
          // code
          if (assertToken_sign(")", sign_ptr) && assertToken_sign("{") &&
              re_node->set_body(parse_code()) && assertToken_sign("}")) {
            return re_node;
          }
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c>
  parse_function_noReturn_define(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    // {函数名} ({参数列表}*) { {code} }
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken_type(WordEnumToken_e::Tid, word_ptr))) {
      if (assertToken_sign("(")) {
        std::shared_ptr<WordItem_c> sign_ptr;
        while (re_node->add(parse_value_define_id())) {
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(",", sign_ptr)) {
            break;
          }
        }
        if (assertToken_sign(")", sign_ptr) && assertToken_sign("{") &&
            re_node->add(parse_code()) && assertToken_sign("}")) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_enum_define_c>
  parse_enum_define(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_enum_define_c>();
    if (assertToken(WordItem_type_c{WordEnumType_e::Tenum}, word_ptr)) {
      if (re_node->set_id(assertToken_type(WordEnumToken_e::Tid)) && assertToken_sign("{")) {
        // ID 列表
        std::shared_ptr<WordItem_c> sign_ptr;
        // TODO: 解析 id <= number>?
        _GEN_WORD(sign);
        while (re_node->add(assertToken_type(WordEnumToken_e::Tid, sign_ptr))) {
          sign_ptr = nullptr;
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(",", sign_ptr)) {
            break;
          }
          sign_ptr = nullptr;
        }
        if (assertToken_sign("}", sign_ptr)) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_class_define(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    auto result = assertToken(WordItem_type_c{WordEnumType_e::Tclass}, word_ptr);
    if (nullptr != result) {
      // TODO: class code
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_type_define(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto enum_ptr = parse_enum_define(word_ptr);
    if (nullptr != enum_ptr) {
      return enum_ptr;
    }
    auto class_ptr = parse_class_define(word_ptr);
    if (nullptr != class_ptr) {
      return class_ptr;
    }
    return nullptr;
  }

  bool analyse() {
    while (true) {
      auto word_ptr = lexicalAnalyse.analyse();
      if (nullptr == word_ptr) {
        return true;
      }
      if (WordEnumToken_e::Toperator == word_ptr->token) {
        auto& sign = word_ptr->toOperator();
        if (WordEnumOperator_e::TSemicolon == sign.value) {
          continue;
        }
      }
      auto result = _REBACK_d(word_ptr, lexicalAnalyse.tokenIndex, parse_type_define,
                              parse_value_define_init, parse_function_define);
      if (nullptr == result) {
        return false;
      }
      root->add(result);
      lexicalAnalyse.currentToken()->printInfo();
    }
    return true;
  }

  // 语法分析结果，抽象语法树根节点
  std::shared_ptr<SyntaxNode_c> root = std::make_shared<SyntaxNode_c>();
  // 词法分析器
  LexicalAnalyse_c lexicalAnalyse{};
};

#undef _GEN_WORD
#undef _GEN_WORD_DEF
#undef _GENERATE_FUN_ITEM_d
#undef _GENERATE_FUN_ITEM
#undef _GENERATE_FUN_ITEM1
#undef _GENERATE_FUN_ITEMN
#undef _REBACK_d
#undef _GEN_VALUE
#undef _PRINT_WORD_PREFIX
#undef _PRINT_NODE_PREFIX
