#pragma once
#include <functional>
#include <list>
#include <memory>

#include "lexical_analyse.h"
#include "magic/macro.h"
#include "util.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 ，给 UtilLog导致展开异常
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
                                                                                                   \
        std::cout << "  │";                                                                        \
      } else {                                                                                     \
        std::cout << "   ";                                                                        \
      }                                                                                            \
      return size + 1;                                                                             \
    });                                                                                            \
  }

enum SyntaxNodeType_e {
  Group, // 分组节点，自身无特殊意义

  Value,
  ValueDefine,
  ValueDefineId,
  ValueSet,
  ValueDefineInit,

  FunctionCall,
  Expr,
  CtrlBreak,
  CtrlContinue,
  CtrlReturn,
  CtrlIfBranch,
  CtrlIf,
  CtrlWhile,
  CtrlFor,
  Code,

  FunctionDefine,
  EnumDefine,
  ClassDefine,
  TypeDefine,
};

enum SyntaxNodeValueClass_e {
  Crude,   // 值类型
  Pointer, // 指针
  Referer, // 引用
};

class SyntaxNodeValueClass_c {
public:
  inline static std::string toName(SyntaxNodeValueClass_e type) {
    switch (type) {
    case SyntaxNodeValueClass_e::Crude: {
      return "Crude";
    } break;
    case SyntaxNodeValueClass_e::Pointer: {
      return "Pointer/*";
    } break;
    case SyntaxNodeValueClass_e::Referer: {
      return "Referer/&";
    } break;
    default:
      break;
    }
    return "";
  }

  inline static void print(SyntaxNodeValueClass_e type) { std::cout << toName(type) << std::endl; }
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

  SyntaxNode_c() : ListNode_c(ListNodeType_e::Syntactic), syntaxType(SyntaxNodeType_e::Group) {}
  SyntaxNode_c(SyntaxNodeType_e type) : ListNode_c(ListNodeType_e::Syntactic), syntaxType(type) {}

  const std::string& name() const override { return HicUtil_c::emptyString; }

  bool add(std::shared_ptr<SyntaxNode_c> ptr) {
    if (nullptr != ptr) {
      children.push_back(ptr);
      return true;
    }
    return false;
  }

  bool add(std::shared_ptr<WordItem_c> ptr) {
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

class SyntaxNode_value_define_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_c() : SyntaxNode_c(SyntaxNodeType_e::ValueDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    value_type->printInfo();
    _PRINT_WORD_PREFIX(true);
    SyntaxNodeValueClass_c::print(valueClass);
  }

  _GEN_VALUE(WordItem_c, value_type);
  SyntaxNodeValueClass_e valueClass = SyntaxNodeValueClass_e::Crude;
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
  _GEN_VALUE(WordItem_c, id);
};

class SyntaxNode_value_define_init_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_init_c() : SyntaxNode_c(SyntaxNodeType_e::ValueDefineInit) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, define_id);
    _PRINT_WORD_PREFIX(false);
    std::cout << "sign ... =" << std::endl;
    _PRINT_NODE_PREFIX(true, data);
  }

  _GEN_VALUE(SyntaxNode_value_define_id_c, define_id);
  // =
  _GEN_VALUE(SyntaxNode_c, data);
};

/**
 * value
 * *value
 * &value
 */
class SyntaxNode_value_c : public SyntaxNode_c {
public:
  SyntaxNode_value_c() : SyntaxNode_c(SyntaxNodeType_e::Value) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    SyntaxNodeValueClass_c::print(valueClass);
    _PRINT_NODE_PREFIX(true, value);
  }

  SyntaxNodeValueClass_e valueClass = SyntaxNodeValueClass_e::Crude;
  _GEN_VALUE(SyntaxNode_c, value); // ID | constexpr
};

class SyntaxNode_function_call_c : public SyntaxNode_c {
public:
  SyntaxNode_function_call_c() : SyntaxNode_c(SyntaxNodeType_e::FunctionCall) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(children.empty());
    name->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_c, name);
};

class SyntaxNode_expr_c : public SyntaxNode_c {
public:
  SyntaxNode_expr_c() : SyntaxNode_c(SyntaxNodeType_e::Expr) {}
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

  _GEN_VALUE(SyntaxNode_expr_c, data);
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
  _GEN_VALUE(SyntaxNode_expr_c, if_expr);
  _GEN_VALUE(SyntaxNode_c, if_body);
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

  _GEN_VALUE(SyntaxNode_expr_c, loop_expr);
  _GEN_VALUE(SyntaxNode_c, body);
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

  _GEN_VALUE(SyntaxNode_expr_c, start_expr);
  _GEN_VALUE(SyntaxNode_expr_c, loop_expr);
  _GEN_VALUE(SyntaxNode_expr_c, loop_end_expr);
  _GEN_VALUE(SyntaxNode_c, body);
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
    name->printInfo();
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
  _GEN_VALUE(WordItem_c, name);
  std::list<std::shared_ptr<SyntaxNode_value_define_id_c>> args;
  _GEN_VALUE(SyntaxNode_c, body);
};

class SyntaxNode_enum_define_c : public SyntaxNode_c {
public:
  SyntaxNode_enum_define_c() : SyntaxNode_c(SyntaxNodeType_e::EnumDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "Enum" << std::endl;
    _PRINT_WORD_PREFIX(false);
    name->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_c, name);
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

  std::shared_ptr<WordItem_c> assertToken(std::shared_ptr<WordItem_c> word_ptr,
                                          const WordItem_c& limit, bool startWith = false) {
    _GEN_WORD(word);
    if (limit.token == word.token) {
      if ((startWith && word.name().starts_with(limit.name())) || (limit.name() == word.name())) {
        return word_ptr;
      }
    }
    if (enableLog_assertToken) {
      SynLog(Twarning, "[assertToken] faild; word: {}, expect: {}", word.toString(),
             limit.toString());
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_type(std::shared_ptr<WordItem_c> word_ptr,
                                               WordEnumToken_e type) {
    _GEN_WORD(word);
    if (type == word.token) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_name(std::shared_ptr<WordItem_c> word_ptr,
                                               const std::string& name, bool startWith = false) {
    _GEN_WORD(word);
    if ((startWith && word.name().starts_with(name)) || (name == word.name())) {
      return word_ptr;
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> assertToken_sign(std::shared_ptr<WordItem_c> word_ptr,
                                               const std::string& sign, bool startWith = false) {
    return assertToken(word_ptr, WordItem_default_c{WordEnumToken_e::Tsign, sign}, startWith);
  }

  // 花括号
  std::shared_ptr<WordItem_c> tryParse_brace(
      std::shared_ptr<WordItem_c> word_ptr,
      const std::function<std::shared_ptr<WordItem_c>(std::shared_ptr<WordItem_c>)>& fun) {
    _GEN_WORD(word)
    const auto left_result = assertToken_sign(word_ptr, "{");
    if (nullptr != left_result) {
      auto result = tryParse_brace(
          nullptr, [&fun](std::shared_ptr<WordItem_c> next_ptr) { return fun(next_ptr); });
      if (nullptr != result) {
        return assertToken_sign(nullptr, "}");
      }
    } else {
      return fun(word_ptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_number_c> parse_constexpr_int(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(word_ptr, WordEnumToken_e::Tnumber);
    if (nullptr != result) {
      return *((std::shared_ptr<WordItem_number_c>*)&result);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_default_c> parse_constexpr_string(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(word_ptr, WordEnumToken_e::Tstring);
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
  parse_value_define(std::shared_ptr<WordItem_c> word_ptr) {
    // value_type
    auto re_node = std::make_shared<SyntaxNode_value_define_c>();
    if (re_node->set_value_type(parse_value_type(word_ptr))) {
      // 指针或引用
      _GEN_WORD_DEF(next);
      const auto sign = assertToken_type(next_ptr, WordEnumToken_e::Tsign);
      if (nullptr != sign) {
        if (sign->name() == "*") {
          re_node->valueClass = SyntaxNodeValueClass_e::Pointer;
        } else if (sign->name() == "&") {
          re_node->valueClass = SyntaxNodeValueClass_e::Referer;
        } else {
          return nullptr;
        }
        return re_node;
      }
      // 下一个 [token] 不是符号，回退一个
      lexicalAnalyse.tokenBack();
      return re_node;
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_value_define_id_c>
  parse_value_define_id(std::shared_ptr<WordItem_c> word_ptr) {
    // value_type
    auto re_node = std::make_shared<SyntaxNode_value_define_id_c>();
    if (re_node->set_value_define(parse_value_define(word_ptr))) {
      // ID
      if (re_node->set_id(assertToken_type(nullptr, WordEnumToken_e::Tid))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_value_define_init_c>
  parse_value_define_init(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_value_define_init_c>();
    if (re_node->set_define_id(parse_value_define_id(word_ptr))) {
      if (assertToken_sign(nullptr, "=")) {
        if (re_node->set_data(parse_expr(nullptr, WordEnumType_e::Tvoid))) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_expr_c> parse_expr(std::shared_ptr<WordItem_c> word_ptr,
                                                WordEnumType_e ret_type) {
    auto re_node = std::make_shared<SyntaxNode_expr_c>();
    std::shared_ptr<WordItem_c> last_ptr;
    int group = 0;
    // 如果不在 () 内，遇到 , 就需要退出
    while (true) {
      _GEN_WORD(word);
      if (word.token == WordEnumToken_e::Tsign) {
        bool doRet = false;
        if (word.name() == "(") {
          ++group;
        } else if (word.name() == ";") {
          doRet = true;
        } else if (word.name() == ")") {
          group--;
          if (group < 0) {
            // 遇到额外多出 ) 才退出
            doRet = true;
          }
        } else if (word.name() == "," && group == 0) {
          doRet = true;
        }
        if (doRet) {
          lexicalAnalyse.tokenBack();
          if (re_node->isEmpty()) {
            return nullptr;
          }
          return re_node;
        }
      }
      re_node->add(word_ptr);
      last_ptr = word_ptr;
      word_ptr = nullptr;
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_expr_c> parse_expr_void(std::shared_ptr<WordItem_c> word_ptr) {
    return parse_expr(word_ptr, WordEnumType_e::Tvoid);
  }

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_break(std::shared_ptr<WordItem_c> word_ptr) {
    auto ptr = assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tbreak});
    if (nullptr != ptr) {
      return SyntaxNode_c::make_node(ptr);
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_continue(std::shared_ptr<WordItem_c> word_ptr) {
    auto ptr = assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tcontinue});
    if (nullptr != ptr) {
      return SyntaxNode_c::make_node(ptr);
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_ctrl_return_c>
  parse_code_ctrl_return(std::shared_ptr<WordItem_c> word_ptr, WordEnumType_e ret_type) {
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Treturn})) {
      auto re_node = std::make_shared<SyntaxNode_ctrl_return_c>();
      if (re_node->set_data(parse_expr(nullptr, ret_type))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_ctrl_return_c>
  parse_code_ctrl_return_void(std::shared_ptr<WordItem_c> word_ptr) {
    return parse_code_ctrl_return(word_ptr, WordEnumType_e::Tvoid);
  }

  std::shared_ptr<SyntaxNode_if_c> parse_code_ctrl_if(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_if_c>();
    auto first_node = std::make_shared<SyntaxNode_if_branch_c>();
    re_node->addBranch(first_node);
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tif}) &&
        assertToken_sign(nullptr, "(") &&
        first_node->set_if_expr(parse_expr(nullptr, WordEnumType_e::Tbool)) &&
        assertToken_sign(nullptr, ")") && assertToken_sign(nullptr, "{")) {
      // if (expr) { code }
      _GEN_WORD_DEF(sign);
      if (assertToken_sign(sign_ptr, "}")) {
        // 空代码块 {}
        return re_node;
      }
      if (first_node->set_if_body(parse_code(sign_ptr)) && assertToken_sign(nullptr, "}")) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_while_c> parse_code_ctrl_while(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_while_c>();
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Twhile}) &&
        assertToken_sign(nullptr, "(") &&
        re_node->set_loop_expr(parse_expr(nullptr, WordEnumType_e::Tbool)) &&
        assertToken_sign(nullptr, ")") && assertToken_sign(nullptr, "{")) {
      // while (expr) { code }
      _GEN_WORD_DEF(sign);
      if (assertToken_sign(sign_ptr, "}")) {
        // 空代码块 {}
        return re_node;
      }
      if (re_node->set_body(parse_code(nullptr)) && assertToken_sign(nullptr, "}")) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_for_c> parse_code_ctrl_for(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_for_c>();
    if (assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tfor})) {
      if (assertToken_sign(nullptr, "(") && re_node->set_start_expr(parse_expr_void(nullptr)) &&
          re_node->set_loop_expr(parse_expr(nullptr, WordEnumType_e ::Tbool)) &&
          re_node->set_loop_end_expr(parse_expr_void(nullptr)) && assertToken_sign(nullptr, ")")) {
        // for (expr;expr;expr) { code }
        if (assertToken_sign(nullptr, "{")) {
          _GEN_WORD_DEF(sign);
          if (assertToken_sign(sign_ptr, "}")) {
            // 空代码块 {}
            return re_node;
          }
          if (re_node->set_body(parse_code(nullptr)) && assertToken_sign(nullptr, "}")) {
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
  std::shared_ptr<SyntaxNode_c> parse_code(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    std::shared_ptr<SyntaxNode_c> result;
    if (enableLog_parseCode) {
      SynLog(Tdebug, "parse_code -----vvv------ ");
    }
    do {
      _GEN_WORD(word);
      if (word.token == WordEnumToken_e::Tsign) {
        if (word.name() == ";") {
          word_ptr = nullptr;
          continue;
        } else if (word.name() == "}") {
          lexicalAnalyse.tokenBack();
          break;
        }
      }
      // word 非终结符
      int tempIndex = lexicalAnalyse.tokenIndex;
      result = _REBACK_d(word_ptr, tempIndex, parse_value_define_init, parse_function_call,
                         parse_code_ctrl_if, parse_code_ctrl_while, parse_code_ctrl_for,
                         parse_code_ctrl_return_void, parse_expr_void);
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
  parse_function_call(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_function_call_c>();
    if (re_node->set_name(assertToken_type(word_ptr, WordEnumToken_e::Tid))) {
      if (assertToken_sign(nullptr, "(")) {
        std::shared_ptr<WordItem_c> sign_ptr;
        // 参数列表
        while (true) {
          if (false == re_node->add(parse_expr(nullptr, WordEnumType_e::Tvoid))) {
            break;
          }
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(sign_ptr, ",")) {
            break;
          }
          sign_ptr = nullptr;
        }
        if (assertToken_sign(sign_ptr, ")")) {
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
      if (re_node->set_name(assertToken_type(nullptr, WordEnumToken_e::Tid))) {
        // 参数列表
        if (assertToken_sign(nullptr, "(")) {
          std::shared_ptr<WordItem_c> sign_ptr;
          _GEN_WORD(sign);
          while (true) {
            if (false == re_node->addArg(parse_value_define_id(sign_ptr))) {
              break;
            }
            sign_ptr = nullptr;
            _GEN_WORD(sign);
            if (nullptr == assertToken_sign(sign_ptr, ",")) {
              break;
            }
            sign_ptr = nullptr;
          }
          // code
          if (assertToken_sign(sign_ptr, ")") && assertToken_sign(nullptr, "{") &&
              re_node->set_body(parse_code(nullptr)) && assertToken_sign(nullptr, "}")) {
            return re_node;
          }
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c>
  parse_function_noReturn_define(std::shared_ptr<WordItem_c> word_ptr) {
    // {函数名} ({参数列表}*) { {code} }
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken_type(word_ptr, WordEnumToken_e::Tid))) {
      if (assertToken_sign(nullptr, "(")) {
        std::shared_ptr<WordItem_c> sign_ptr;
        while (re_node->add(parse_value_define_id(nullptr))) {
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(sign_ptr, ",")) {
            break;
          }
        }
        if (assertToken_sign(sign_ptr, ")") && assertToken_sign(nullptr, "{") &&
            re_node->add(parse_code(nullptr)) && assertToken_sign(nullptr, "}")) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_enum_define_c>
  parse_enum_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_enum_define_c>();
    if (assertToken(word_ptr, WordItem_type_c{WordEnumType_e::Tenum})) {
      if (re_node->set_name(assertToken_type(nullptr, WordEnumToken_e::Tid)) &&
          assertToken_sign(nullptr, "{")) {
        // ID 列表
        std::shared_ptr<WordItem_c> sign_ptr;
        // TODO: 解析 id <= number>?
        _GEN_WORD(sign);
        while (re_node->add(assertToken_type(sign_ptr, WordEnumToken_e::Tid))) {
          sign_ptr = nullptr;
          _GEN_WORD(sign);
          if (nullptr == assertToken_sign(sign_ptr, ",")) {
            break;
          }
          sign_ptr = nullptr;
        }
        if (assertToken_sign(sign_ptr, "}")) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_class_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    auto result = assertToken(word_ptr, WordItem_type_c{WordEnumType_e::Tclass});
    if (nullptr != result) {
      // TODO: class code
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_type_define(std::shared_ptr<WordItem_c> word_ptr) {
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
      auto& word = *word_ptr.get();
      if (word.token == WordEnumToken_e::Tsign && (word.name() == ";")) {
        continue;
      }
      auto result = _REBACK_d(word_ptr, lexicalAnalyse.tokenIndex, parse_type_define,
                              parse_value_define_init, parse_function_define);
      if (nullptr == result) {
        return false;
      }
      root.add(result);
      lexicalAnalyse.currentToken()->printInfo();
    }
    return true;
  }

  SyntaxNode_c root = SyntaxNode_c{};
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