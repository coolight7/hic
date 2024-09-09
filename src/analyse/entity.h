#pragma once

#include <functional>
#include <list>
#include <memory>
#include <stack>

#include "analyse/rule.h"
#include "analyse/util.h"

class WordItem_string_c;
class WordItem_operator_c;
class WordItem_number_c;
class WordItem_ctrl_c;
class WordItem_type_c;
class WordItem_nativeCall_c;

class WordItem_c : public ListNode_c {
public:
  WordItem_c() : ListNode_c(ListNodeType_e::Lexical), token(WordEnumToken_e::Tundefined) {}

  WordItem_c(WordEnumToken_e in_token) : ListNode_c(ListNodeType_e::Lexical), token(in_token) {}
  WordItem_c(const WordItem_c&) = delete;

  const std::string& name() const override { return HicUtil_c::emptyString; }

  void printInfo() const override {
    const auto& str = WordEnumToken_c::toName(token);
    std::cout << str << " ";
    int fill = 10 - str.size();
    if (fill > 0) {
      while (fill-- > 0) {
        std::cout << "-";
      }
    }
    std::cout << " " << name() << std::endl;
  }

  template <typename _T, typename... _Args>
  static std::shared_ptr<WordItem_c> make_shared(_Args... args) {
    _T* ptr = new _T{std::forward<_Args>(args)...};
    return std::shared_ptr<WordItem_c>(ptr);
  }

  WordItem_string_c& toString() const {
    Assert_d(WordEnumToken_e::Tstring == token);
    return *((WordItem_string_c*)this);
  }
  WordItem_string_c& toId() const {
    Assert_d(WordEnumToken_e::Tid == token);
    return *((WordItem_string_c*)this);
  }
  WordItem_operator_c& toOperator() const {
    Assert_d(WordEnumToken_e::Toperator == token);
    return *((WordItem_operator_c*)this);
  }
  WordItem_number_c& toNumber() const {
    Assert_d(WordEnumToken_e::Tnumber == token);
    return *((WordItem_number_c*)this);
  }
  WordItem_ctrl_c& toKeyword() const {
    Assert_d(WordEnumToken_e::Tkeyword == token);
    return *((WordItem_ctrl_c*)this);
  }
  WordItem_type_c& toType() const {
    Assert_d(WordEnumToken_e::Ttype == token);
    return *((WordItem_type_c*)this);
  }
  WordItem_nativeCall_c& toNativeCall() const {
    Assert_d(WordEnumToken_e::TnativeCall == token);
    return *((WordItem_nativeCall_c*)this);
  }

  bool isString() const { return (WordEnumToken_e::Tstring == token); }
  bool isId() const { return (WordEnumToken_e::Tid == token); }
  bool isOperator() const { return (WordEnumToken_e::Toperator == token); }
  bool isNumber() const { return (WordEnumToken_e::Tnumber == token); }
  bool isKeyword() const { return (WordEnumToken_e::Tkeyword == token); }
  bool isType() const { return (WordEnumToken_e::Ttype == token); }
  bool isNativeCall() const { return (WordEnumToken_e::TnativeCall == token); }

  virtual ~WordItem_c() {}

  std::string toFormat() const {
    return std::format("[{}] {}", WordEnumToken_c::toName(token), name());
  }

  WordEnumToken_e token;
};

class WordItem_string_c : public WordItem_c {
public:
  WordItem_string_c(const std::string_view& in_value)
      : WordItem_c(WordEnumToken_e::Tstring), value(in_value) {}

  const std::string& name() const override { return value; }

  // std::shared_ptr<SyntaxNode_value_define_c> returnType() const override {
  //   auto node = std::make_shared<SyntaxNode_value_define_c>();
  //   node->set_value_type(this);
  //   return node;
  // }

  std::string value;
};

class WordItem_id_c : public WordItem_c {
public:
  WordItem_id_c(const std::string_view& in_id) : WordItem_c(WordEnumToken_e::Tid), id(in_id) {}

  const std::string& name() const override { return id; }

  std::string id;
};

class WordItem_operator_c : public WordItem_c {
public:
  WordItem_operator_c(WordEnumOperator_e in_value)
      : WordItem_c(WordEnumToken_e::Toperator), value(in_value) {}

  const std::string& name() const override { return toSign(value); }

  // `R"()"` fix warning `trigraph ??= ignored, use -trigraphs to enable`
  inline static constexpr std::array<const std::string, 48> signlist = {
      "[Undefine]", "",      "expr++", "expr--", "(",      ")",      "[",  "]",   ".",  "?.",
      "Level1",     "!expr", "~expr",  "++expr", "--expr", "Level2", "*",  "/",   "%",  "+",
      "-",          "<<",    ">>",     "&",      "|",      ">=",     ">",  "<=",  "<",  "==",
      "!=",         "&&",    "||",     "??",     "? :",    "=",      "|=", "&=",  "*=", "/=",
      "+=",         "-=",    R"(??=)", "{",      "}",      ";",      ",",  "END",
  };

  inline static constexpr const std::string& toSign(WordEnumOperator_e value) {
    static_assert(WordEnumOperator_c::namelist.size() == signlist.size());
    assert(WordEnumOperator_c::namelist[WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel1)] ==
           signlist[WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel1)]);
    assert(WordEnumOperator_c::namelist[WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel2)] ==
           signlist[WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel2)]);
    assert(WordEnumOperator_c::namelist[WordEnumOperator_c::toInt(WordEnumOperator_e::TEND)] ==
           signlist[WordEnumOperator_c::toInt(WordEnumOperator_e::TEND)]);
    return signlist[WordEnumOperator_c::toInt(value)];
  }

  inline static constexpr WordEnumOperator_e toEnum(const std::string_view str) {
    if ("++" == str) {
      return WordEnumOperator_e::TEndAddAdd;
    }
    if ("--" == str) {
      return WordEnumOperator_e::TEndSubSub;
    }
    if ("(" == str) {
      return WordEnumOperator_e::TLeftCurvesGroup;
    }
    if (")" == str) {
      return WordEnumOperator_e::TRightCurvesGroup;
    }
    if ("[" == str) {
      return WordEnumOperator_e::TLeftSquareGroup;
    }
    if ("]" == str) {
      return WordEnumOperator_e::TRightSquareGroup;
    }
    if ("." == str) {
      return WordEnumOperator_e::TDot;
    }
    if ("?." == str) {
      return WordEnumOperator_e::TNullDot;
    }
    if ("!" == str) {
      return WordEnumOperator_e::TNot;
    }
    if ("~" == str) {
      return WordEnumOperator_e::TShift;
    }
    if ("++" == str) {
      return WordEnumOperator_e::TStartAddAdd;
    }
    if ("--" == str) {
      return WordEnumOperator_e::TStartSubSub;
    }
    if ("*" == str) {
      return WordEnumOperator_e::TMulti;
    }
    if ("/" == str) {
      return WordEnumOperator_e::TDivision;
    }
    if ("%" == str) {
      return WordEnumOperator_e::TPercent;
    }
    if ("+" == str) {
      return WordEnumOperator_e::TAdd;
    }
    if ("-" == str) {
      return WordEnumOperator_e::TSub;
    }
    if ("<<" == str) {
      return WordEnumOperator_e::TBitLeftMove;
    }
    if (">>" == str) {
      return WordEnumOperator_e::TBitRightMove;
    }
    if ("&" == str) {
      return WordEnumOperator_e::TBitAnd;
    }
    if ("|" == str) {
      return WordEnumOperator_e::TBitOr;
    }
    if (">=" == str) {
      return WordEnumOperator_e::TGreaterOrEqual;
    }
    if (">" == str) {
      return WordEnumOperator_e::TGreater;
    }
    if ("<=" == str) {
      return WordEnumOperator_e::TLessOrEqual;
    }
    if ("<" == str) {
      return WordEnumOperator_e::TLess;
    }
    if ("==" == str) {
      return WordEnumOperator_e::TEqual;
    }
    if ("!=" == str) {
      return WordEnumOperator_e::TNotEqual;
    }
    if ("&&" == str) {
      return WordEnumOperator_e::TAnd;
    }
    if ("||" == str) {
      return WordEnumOperator_e::TOr;
    }
    if ("??" == str) {
      return WordEnumOperator_e::TNullMerge;
    }
    if ("? :" == str) {
      return WordEnumOperator_e::TIfElse;
    }
    if ("=" == str) {
      return WordEnumOperator_e::TSet;
    }
    if ("*=" == str) {
      return WordEnumOperator_e::TSetMulti;
    }
    if ("/=" == str) {
      return WordEnumOperator_e::TSetDivision;
    }
    if ("+=" == str) {
      return WordEnumOperator_e::TSetAdd;
    }
    if ("-=" == str) {
      return WordEnumOperator_e::TSetSub;
    }
    if (str == R"(??=)") {
      // `R"()"` fix warning `trigraph ??= ignored, use -trigraphs to enable`
      return WordEnumOperator_e::TSetNullMerge;
    }
    if ("{" == str) {
      return WordEnumOperator_e::TLeftFlowerGroup;
    }
    if ("}" == str) {
      return WordEnumOperator_e::TRightFlowerGroup;
    }
    if (";" == str) {
      return WordEnumOperator_e::TSemicolon;
    }
    if ("," == str) {
      return WordEnumOperator_e::TComma;
    }
    Assert_d(true == false, "未知操作符: {}", str);
    return WordEnumOperator_e::TUndefined;
  }

  inline static int toLevel(WordEnumOperator_e type) {
    Assert_d(type != WordEnumOperator_e::TUndefined && type != WordEnumOperator_e::TLevel1 &&
             type != WordEnumOperator_e::TLevel2);
    int index = WordEnumOperator_c::toInt(type);
    if (index < WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel1)) {
      return 1;
    }
    if (index < WordEnumOperator_c::toInt(WordEnumOperator_e::TLevel2)) {
      return 2;
    }
    return index;
  }

  /**
   * ## [left] 优先级是否大于 [right]
   * - result > 0  : [left] 的优先级小于 [right]
   * - result == 0 : [left] 的优先级等于 [right]，注意此时 [left] 和 [right] 不一定是相同
   * 符号，部分符号的优先级是相等的。
   * - result < 0  : [left] 的优先级大于 [right]
   */
  inline static int compare(WordEnumOperator_e left, WordEnumOperator_e right) {
    return toLevel(left) - toLevel(right);
  }

  WordEnumOperator_e value;
};

class WordItem_number_c : public WordItem_c {
public:
  WordItem_number_c(long long in_value) : WordItem_c(WordEnumToken_e::Tnumber), value(in_value) {
    name_ = std::to_string(in_value);
  }

  const std::string& name() const override { return name_; }

  std::string name_;
  long long value;
};

class WordItem_ctrl_c : public WordItem_c {
public:
  WordItem_ctrl_c(WordEnumCtrl_e in_value)
      : WordItem_c(WordEnumToken_e::Tkeyword), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumCtrl_c::namelist.size());
    return WordEnumCtrl_c::namelist[value];
  }

  // index
  WordEnumCtrl_e value;
};

class WordItem_type_c : public WordItem_c {
public:
  WordItem_type_c(WordEnumType_e in_value) : WordItem_c(WordEnumToken_e::Ttype), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumType_c::namelist.size());
    return WordEnumType_c::namelist[value];
  }

  WordEnumType_e value;
};

class WordItem_nativeCall_c : public WordItem_c {
public:
  WordItem_nativeCall_c(WordEnumNativeCall_e in_value)
      : WordItem_c(WordEnumToken_e::TnativeCall), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumNativeCall_c::namelist.size());
    return WordEnumNativeCall_c::namelist[value];
  }

  // index
  WordEnumNativeCall_e value;
};

// syntax ---
#include "magic/macro.h"

GENERATE_ENUM(SyntaxNodeType,
              Normal, // 分组节点，自身无特殊意义
              Group,  // {}，隔离符号范围
              ValueDefine, ValueDefineId, ValueDefineInit, FunctionCall, Operator, CtrlReturn,
              CtrlIfBranch, CtrlIf, CtrlWhile, CtrlFor, FunctionDefine, EnumDefine, ClassDefine, );

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

enum SyntaxNodeValueClass_e {
  Crude,   // 值类型
  Pointer, // 指针
  Referer, // 引用
};

class SyntaxNode_value_define_c;

/**
 * ## 语法树节点
 */
class SyntaxNode_c : public ListNode_c {
public:
  template <typename... _Args>
  inline static std::shared_ptr<SyntaxNode_c> make_node(std::shared_ptr<_Args>... args) {
    auto re_ptr = std::make_shared<SyntaxNode_c>();
    re_ptr->children.push_back(args...);
    return re_ptr;
  }

  SyntaxNode_c() : ListNode_c(ListNodeType_e::Syntactic), syntaxType(SyntaxNodeType_e::TNormal) {}
  SyntaxNode_c(SyntaxNodeType_e type) : ListNode_c(ListNodeType_e::Syntactic), syntaxType(type) {}

  const std::string& name() const override { return HicUtil_c::emptyString; }

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

  bool add(std::shared_ptr<ListNode_c> ptr) {
    if (nullptr != ptr) {
      children.push_back(ptr);
      return true;
    }
    return false;
  }

  bool isEmpty() { return children.empty(); }

  SyntaxNodeType_e syntaxType;
  std::list<std::shared_ptr<ListNode_c>> children{};
};

/**
 * - 额外携带符号表，限定符号声明范围
 */
class SyntaxNode_group_c : public SyntaxNode_c {
public:
  SyntaxNode_group_c(SyntaxNodeType_e type = SyntaxNodeType_e::TGroup) : SyntaxNode_c(type) {}

  std::shared_ptr<SymbolTable> symbolTable;
};

class SyntaxNode_value_define_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_c() : SyntaxNode_c(SyntaxNodeType_e::TValueDefine) {}

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

  // TODO: 比较两个类型定义是否相同
  inline static bool compare(std::shared_ptr<SyntaxNode_value_define_c> left,
                             std::shared_ptr<SyntaxNode_value_define_c> right) {
    return true;
  }

  bool isBoolValue() const {
    return (0 == pointer && value_type->isType() &&
            WordEnumType_e::Tbool == value_type->toType().value);
  }

  bool isIntValue() const {
    return (0 == pointer && value_type->isType() &&
            WordEnumType_e::Tint == value_type->toType().value);
  }

  // Type | ID
  _GEN_VALUE(WordItem_c, value_type);
  bool isReferer = false; // 是否是引用类型
  size_t pointer = 0;     // 指针层数
};

class SyntaxNode_value_define_id_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_id_c() : SyntaxNode_c(SyntaxNodeType_e::TValueDefineId) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, value_define);
    _PRINT_WORD_PREFIX(true);
    id->printInfo();
  }

  _GEN_VALUE(SyntaxNode_value_define_c, value_define);
  _GEN_VALUE(WordItem_id_c, id);

  std::shared_ptr<SymbolItem_value_c> symbol;
};

class SyntaxNode_value_define_init_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_init_c() : SyntaxNode_c(SyntaxNodeType_e::TValueDefineInit) {}

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
  SyntaxNode_function_call_c() : SyntaxNode_c(SyntaxNodeType_e::TFunctionCall) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(children.empty());
    id->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  std::shared_ptr<SyntaxNode_value_define_c> returnType() const override { return return_type; }

  _GEN_VALUE(WordItem_id_c, id);

  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
  std::shared_ptr<SymbolItem_function_c> symbol;
};

class SyntaxNode_operator_c : public SyntaxNode_c {
public:
  SyntaxNode_operator_c(WordEnumOperator_e in_op)
      : SyntaxNode_c(SyntaxNodeType_e::TOperator), oper(in_op) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    if (WordEnumOperator_e::TNone != oper) {
      _PRINT_WORD_PREFIX(children.empty());
      std::cout << "operator -- " << WordItem_operator_c::toSign(oper) << std::endl;
    }
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  std::shared_ptr<SyntaxNode_value_define_c> returnType() const override { return return_type; }

  bool isReturnBool() { return (nullptr != return_type && return_type->isBoolValue()); }

  // 操作符类型
  WordEnumOperator_e oper;
  // 操作数在 [children] 中

  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
};

class SyntaxNode_ctrl_return_c : public SyntaxNode_c {
public:
  SyntaxNode_ctrl_return_c() : SyntaxNode_c(SyntaxNodeType_e::TCtrlReturn) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "return" << std::endl;
    _PRINT_NODE_PREFIX(true, data);
  }

  _GEN_VALUE(SyntaxNode_operator_c, data);

  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
};

class SyntaxNode_if_branch_c : public SyntaxNode_c {
public:
  SyntaxNode_if_branch_c() : SyntaxNode_c(SyntaxNodeType_e::TCtrlIfBranch) {}

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
  SyntaxNode_if_c() : SyntaxNode_c(SyntaxNodeType_e::TCtrlIf) {}

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
  SyntaxNode_while_c() : SyntaxNode_c(SyntaxNodeType_e::TCtrlWhile) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "while" << std::endl;
    _PRINT_NODE_PREFIX(false, loop_expr);
    _PRINT_NODE_PREFIX(true, body);
  }

  // while (loop_expr) { body }
  _GEN_VALUE(SyntaxNode_operator_c, loop_expr);
  _GEN_VALUE(SyntaxNode_group_c, body);
};

class SyntaxNode_for_c : public SyntaxNode_group_c {
public:
  SyntaxNode_for_c() : SyntaxNode_group_c(SyntaxNodeType_e::TCtrlFor) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "for" << std::endl;
    _PRINT_NODE_PREFIX(false, start_expr);
    _PRINT_NODE_PREFIX(false, loop_expr);
    _PRINT_NODE_PREFIX(false, loop_end_expr);
    _PRINT_NODE_PREFIX(true, body);
  }

  // for (start_expr; loop_expr; loop_end_expr) { body }
  _GEN_VALUE(SyntaxNode_operator_c, start_expr);
  _GEN_VALUE(SyntaxNode_operator_c, loop_expr);
  _GEN_VALUE(SyntaxNode_operator_c, loop_end_expr);
  _GEN_VALUE(SyntaxNode_group_c, body);
};

class SyntaxNode_function_define_c : public SyntaxNode_group_c {
public:
  SyntaxNode_function_define_c() : SyntaxNode_group_c(SyntaxNodeType_e::TFunctionDefine) {}

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

  std::shared_ptr<SyntaxNode_value_define_c> returnType() const override { return return_type; }

  bool addArg(std::shared_ptr<SyntaxNode_value_define_id_c> arg) {
    if (nullptr == arg) {
      return false;
    }
    args.push_back(arg);
    return true;
  }

  // [args] 和 [body] 在同一符号范围
  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
  _GEN_VALUE(WordItem_id_c, id);
  std::list<std::shared_ptr<SyntaxNode_value_define_id_c>> args;
  _GEN_VALUE(SyntaxNode_c, body);

  std::shared_ptr<SymbolItem_function_c> symbol;
};

class SyntaxNode_enum_define_c : public SyntaxNode_group_c {
public:
  SyntaxNode_enum_define_c() : SyntaxNode_group_c(SyntaxNodeType_e::TEnumDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "Enum" << std::endl;
    _PRINT_WORD_PREFIX(false);
    id->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_id_c, id);
};