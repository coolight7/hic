#pragma once

#include <functional>
#include <list>
#include <memory>
#include <stack>

#include "rule.h"
#include "src/util.h"

// marco ---
#include "src/magic/macro.h"

GENERATE_ENUM(SyntaxNodeType,
              Normal,   // 分组节点，自身无特殊意义
              Group,    // {}，隔离符号范围
              Transfer, // 传递返回值
              ValueDefine, ValueDefineId, ValueDefineInit, NativeFunctionCall, UserFunctionCall,
              Operator, CtrlReturn, CtrlIfBranch, CtrlIf, CtrlWhile, CtrlFor, NativeFunctionDefine,
              UserFunctionDefine, EnumDefine, ClassDefine);

#include "src/magic/unset_macro.h"

#define _GEN_VALUE(type, name)                                                                     \
  bool set_##name(std::shared_ptr<type> in_ptr) {                                                  \
    if (nullptr == in_ptr) {                                                                       \
      return false;                                                                                \
    }                                                                                              \
    name = in_ptr;                                                                                 \
    return true;                                                                                   \
  }                                                                                                \
  template <typename... _ARGS> std::shared_ptr<type>& make_##name(_ARGS... args) {                 \
    name = std::make_shared<type>(args...);                                                        \
    return name;                                                                                   \
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

// word
class WordItem_c;
class WordItem_string_c;
class WordItem_id_c;
class WordItem_number_c;
class WordItem_operator_c;
class WordItem_ctrl_c;
class WordItem_type_c;
class WordItem_value_c;
class WordItem_nativeCall_c;

// syn ---
class SyntaxNode_value_define_c;
class SyntaxNode_function_define_base_c;
class SyntaxNode_function_call_base_c;
class SyntaxNode_enum_define_c;

// semantic ---
class SymbolItem_c;
class SymbolItem_value_c;
class SymbolItem_function_c;
class SymbolItem_enum_c;

enum TypeLimit_e {
  Constexpr,      // 编译期常量表达式
  ValueConstexpr, // 编译期已知值的变量
  Final,          // 不允许修改
  Normal,         // 普通类型
};

class Type_c {
public:
  // 可转为任意类型的指针
  inline static const size_t anyPointer = 777;

  Type_c(WordEnumType_e in_type) : type(in_type) {}
  Type_c(WordEnumType_e in_type, TypeLimit_e in_limit) : type(in_type), limit(in_limit) {}
  Type_c(WordEnumType_e in_type, TypeLimit_e in_limit, std::shared_ptr<WordItem_c> in_word)
      : type(in_type), limit(in_limit), word(in_word) {}

  virtual void debugPrint(const size_t tab = 1,
                          std::function<size_t()> onOutPrefix = nullptr) const;
  virtual void printInfo() const { debugPrint(); }
  virtual const std::string& name() const { return WordEnumType_c::toName(type); }

  bool isAnyPointer() const { return (anyPointer == pointer); }

  bool isBoolValue() const { return (0 == pointer && WordEnumType_e::Tbool == type); }

  bool isIntValue() const {
    if (0 == pointer) {
      switch (type) {
      case WordEnumType_e::Tint:
      case WordEnumType_e::Tint64:
        return true;
      default:
        return false;
      }
    }
    return false;
  }

  bool isFloatValue() const {
    if (0 == pointer) {
      switch (type) {
      case WordEnumType_e::Tfloat:
      case WordEnumType_e::Tfloat64:
        return true;
      default:
        return false;
      }
    }
    return false;
  }

  bool isConstexpr() const { return (TypeLimit_e::Constexpr == limit); }

  bool canModify() const { return (TypeLimit_e::Normal == limit); }

  size_t size() const;

  // 检查变量类型
  static bool compare(std::shared_ptr<Type_c> left, std::shared_ptr<Type_c> right);

  std::shared_ptr<WordItem_c> word;
  WordEnumType_e type;
  TypeLimit_e limit;      // 类型限制
  bool isReferer = false; // 是否是引用类型
  size_t pointer = 0;     // 指针层数
};

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

  virtual std::shared_ptr<WordItem_c> copy() const { return nullptr; }

  template <typename _T, typename... _Args>
  static std::shared_ptr<_T> make_shared(_Args&&... args) {
    return std::make_shared<_T>(std::forward<_Args>(args)...);
  }

  WordItem_string_c& toString() const {
    Assert_d(WordEnumToken_e::Tstring == token);
    return *((WordItem_string_c*)this);
  }
  WordItem_id_c& toId() const {
    Assert_d(WordEnumToken_e::Tid == token);
    return *((WordItem_id_c*)this);
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
  WordItem_value_c& toValue() const {
    Assert_d(WordEnumToken_e::Tvalue == token);
    return *((WordItem_value_c*)this);
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

  std::string formatInfo() const {
    return std::format("[{}] {}", WordEnumToken_c::toName(token), name());
  }

  WordEnumToken_e token;
};

class WordItem_string_c : public WordItem_c {
public:
  WordItem_string_c(const std::string_view& in_value)
      : WordItem_c(WordEnumToken_e::Tstring), value(in_value) {}

  const std::string& name() const override { return value; }

  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  std::string value;
  _GEN_VALUE(Type_c, return_type);
};

class WordItem_id_c : public WordItem_c {
public:
  WordItem_id_c(const std::string_view& in_id) : WordItem_c(WordEnumToken_e::Tid), id(in_id) {}

  const std::string& name() const override { return id; }

  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  std::string id;
  _GEN_VALUE(Type_c, return_type);
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
  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  std::string name_;
  long long value;

  _GEN_VALUE(Type_c, return_type);
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
  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  WordEnumType_e value;
  _GEN_VALUE(Type_c, return_type);
};

class WordItem_value_c : public WordItem_c {
public:
  WordItem_value_c(WordEnumValue_e in_value)
      : WordItem_c(WordEnumToken_e::Tvalue), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumType_c::namelist.size());
    return WordEnumValue_c::namelist[value];
  }

  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  WordEnumValue_e value;
  _GEN_VALUE(Type_c, return_type);
};

class WordItem_nativeCall_c : public WordItem_c {
public:
  WordItem_nativeCall_c(WordEnumNativeCall_e in_value)
      : WordItem_c(WordEnumToken_e::TnativeCall), value(in_value) {}

  const std::string& name() const override {
    Assert_d(value >= 0 && value < WordEnumNativeCall_c::namelist.size());
    return WordEnumNativeCall_c::namelist[value];
  }

  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  // index
  WordEnumNativeCall_e value;
  _GEN_VALUE(Type_c, return_type);
};

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
      }
    }
    return nullptr;
  }

  inline static std::shared_ptr<SymbolItem_function_c>
  toFunction(std::shared_ptr<SymbolItem_c> ptr) {
    if (nullptr != ptr) {
      switch (ptr->symbolType) {
      case SymbolType_e::TFunction: {
        return HicUtil_c::toType<SymbolItem_function_c>(ptr);
      } break;
      }
    }
    return nullptr;
  }

  inline static std::shared_ptr<SymbolItem_enum_c> toEnum(std::shared_ptr<SymbolItem_c> ptr) {
    if (nullptr != ptr) {
      switch (ptr->symbolType) {
      case SymbolType_e::TEnum: {
        return HicUtil_c::toType<SymbolItem_enum_c>(ptr);
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

  size_t size();

  std::shared_ptr<Type_c> type;
  // address 相对地址
  long long address = 0;
  std::list<std::shared_ptr<WordItem_c>> refs;
};

// 函数符号定义
class SymbolItem_function_c : public SymbolItem_c {
public:
  SymbolItem_function_c() : SymbolItem_c(SymbolType_e::TFunction) {}

  bool hasType() const override { return (nullptr != type); }

  // 检查函数类型
  template <typename... _ARGS>
  bool checkFunType(const Type_c&& return_type, const _ARGS&&... args) {
    return true;
  }

  // TODO: 检查传入参数是否匹配
  bool checkFunArgs(std::list<std::shared_ptr<ListNode_c>>& args);

  std::shared_ptr<SyntaxNode_function_define_base_c> type;
  long long address = 0;
  std::list<std::shared_ptr<SyntaxNode_function_call_base_c>> refs;
};

// 枚举符号定义
class SymbolItem_enum_c : public SymbolItem_c {
public:
  SymbolItem_enum_c() : SymbolItem_c(SymbolType_e::TEnum) {}

  bool hasType() const override { return (nullptr != type); }

  std::shared_ptr<SyntaxNode_enum_define_c> type;
};

// syntax ---

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

class SyntaxNode_transfer_c : public SyntaxNode_c {
public:
  SyntaxNode_transfer_c(SyntaxNodeType_e type = SyntaxNodeType_e::TTransfer) : SyntaxNode_c(type) {}

  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
};

/**
 * - 额外携带符号表，限定符号声明范围
 */
class SyntaxNode_group_c : public SyntaxNode_c {
public:
  SyntaxNode_group_c(SyntaxNodeType_e type = SyntaxNodeType_e::TGroup) : SyntaxNode_c(type) {}

  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  std::shared_ptr<SymbolTable> symbolTable;

  _GEN_VALUE(Type_c, return_type);
};

class SyntaxNode_value_define_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_c() : SyntaxNode_c(SyntaxNodeType_e::TValueDefine) {}
  // 浅拷贝，拥有同样的 [value_type]
  SyntaxNode_value_define_c(const SyntaxNode_value_define_c& crude)
      : SyntaxNode_c(SyntaxNodeType_e::TValueDefine) {
    value_type = crude.value_type;
  }

  const std::string& name() const override { return value_type->name(); }

  void debugPrint(const size_t tab = 1, std::function<size_t()> onOutPrefix = nullptr) const {
    value_type->debugPrint(tab, onOutPrefix);
  }

  std::shared_ptr<Type_c> returnType() const override { return value_type; }

  // Type | ID
  _GEN_VALUE(Type_c, value_type);
};

class SyntaxNode_operator_c : public SyntaxNode_c {
public:
  SyntaxNode_operator_c(WordEnumOperator_e in_op)
      : SyntaxNode_c(SyntaxNodeType_e::TOperator), oper(in_op) {}

  const std::string& name() const override { return WordEnumOperator_c::toName(oper); }

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    if (WordEnumOperator_e::TNone != oper) {
      _PRINT_WORD_PREFIX(children.empty());
      std::cout << "operator -- " << WordItem_operator_c::toSign(oper) << std::endl;
    }
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  std::shared_ptr<Type_c> returnType() const override { return return_type; }

  bool isReturnBool() { return (nullptr != return_type && return_type->isBoolValue()); }

  // 操作符类型
  WordEnumOperator_e oper;
  // 操作数在 [children] 中

  _GEN_VALUE(Type_c, return_type);
};

class SyntaxNode_value_define_id_c : public SyntaxNode_c {
public:
  SyntaxNode_value_define_id_c() : SyntaxNode_c(SyntaxNodeType_e::TValueDefineId) {}

  const std::string& name() const override { return id->name(); }

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, value_define);
    _PRINT_WORD_PREFIX(true);
    id->printInfo();
  }

  std::shared_ptr<Type_c> returnType() const override { return value_define->returnType(); }

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

  std::shared_ptr<Type_c> returnType() const override { return define_id->returnType(); }

  _GEN_VALUE(SyntaxNode_value_define_id_c, define_id);
  // =
  _GEN_VALUE(SyntaxNode_operator_c, data);
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

  std::shared_ptr<Type_c> returnType() const override {
    if (nullptr == data) {
      return nullptr;
    }
    return data->returnType();
  }

  _GEN_VALUE(SyntaxNode_operator_c, data);
};

class SyntaxNode_if_branch_c : public SyntaxNode_c {
public:
  SyntaxNode_if_branch_c() : SyntaxNode_c(SyntaxNodeType_e::TCtrlIfBranch) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_NODE_PREFIX(false, if_expr);
    _PRINT_NODE_PREFIX(true, if_body);
  }

  std::shared_ptr<Type_c> returnType() const override { return if_body->returnType(); }

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

  std::shared_ptr<Type_c> returnType() const override {
    for (const auto& item : branchs) {
      // TODO: 检查每一个都符合返回要求
      auto result = item->returnType();
      if (nullptr != result) {
        return result;
      }
    }
    return nullptr;
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

  std::shared_ptr<Type_c> returnType() const override { return body->returnType(); }

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

  std::shared_ptr<Type_c> returnType() const override { return body->returnType(); }

  // for (start_expr; loop_expr; loop_end_expr) { body }
  _GEN_VALUE(SyntaxNode_operator_c, start_expr);
  _GEN_VALUE(SyntaxNode_operator_c, loop_expr);
  _GEN_VALUE(SyntaxNode_operator_c, loop_end_expr);
  _GEN_VALUE(SyntaxNode_c, body);
};

class SyntaxNode_function_define_native_c;
class SyntaxNode_function_define_user_c;

class SyntaxNode_function_define_base_c : public SyntaxNode_group_c {
protected:
  friend SyntaxNode_function_define_native_c;
  friend SyntaxNode_function_define_user_c;

  SyntaxNode_function_define_base_c(SyntaxNodeType_e type) : SyntaxNode_group_c(type) {}

public:
  std::shared_ptr<Type_c> returnType() const override { return return_type->returnType(); }

  bool addArg(std::shared_ptr<SyntaxNode_value_define_id_c> arg) {
    if (nullptr == arg) {
      return false;
    }
    args.push_back(arg);
    return true;
  }

  // [args] 和 [body] 在同一符号范围
  _GEN_VALUE(SyntaxNode_value_define_c, return_type);
  std::list<std::shared_ptr<SyntaxNode_value_define_id_c>> args;

  std::shared_ptr<SymbolItem_function_c> symbol;
};

class SyntaxNode_function_define_native_c : public SyntaxNode_function_define_base_c {
public:
  SyntaxNode_function_define_native_c()
      : SyntaxNode_function_define_base_c(SyntaxNodeType_e::TNativeFunctionDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "Native-Function" << std::endl;
    _PRINT_NODE_PREFIX(false, return_type);
    _PRINT_WORD_PREFIX(false);
    id->printInfo();
    for (const auto& item : args) {
      _PRINT_NODE_PREFIX(false, item);
    }
  }

  _GEN_VALUE(WordItem_nativeCall_c, id);
};

class SyntaxNode_function_define_user_c : public SyntaxNode_function_define_base_c {
public:
  SyntaxNode_function_define_user_c()
      : SyntaxNode_function_define_base_c(SyntaxNodeType_e::TUserFunctionDefine) {}

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(false);
    std::cout << "User-Function" << std::endl;
    _PRINT_NODE_PREFIX(false, return_type);
    _PRINT_WORD_PREFIX(false);
    id->printInfo();
    for (const auto& item : args) {
      _PRINT_NODE_PREFIX(false, item);
    }
    _PRINT_NODE_PREFIX(true, body);
  }

  _GEN_VALUE(WordItem_id_c, id);
  _GEN_VALUE(SyntaxNode_c, body);
};

class SyntaxNode_function_call_base_c : public SyntaxNode_c {
public:
  SyntaxNode_function_call_base_c(SyntaxNodeType_e type) : SyntaxNode_c(type) {}

  std::shared_ptr<Type_c> returnType() const override { return symbol->type->returnType(); }

  std::shared_ptr<SymbolItem_function_c> symbol;
};

class SyntaxNode_function_call_c : public SyntaxNode_function_call_base_c {
public:
  SyntaxNode_function_call_c()
      : SyntaxNode_function_call_base_c(SyntaxNodeType_e::TUserFunctionCall) {}

  const std::string& name() const override { return id->name(); }

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(children.empty());
    id->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_id_c, id);
  // std::list<SyntaxNode_operator_c> children; 函数参数
};

class SyntaxNode_native_call_c : public SyntaxNode_function_call_base_c {
public:
  SyntaxNode_native_call_c()
      : SyntaxNode_function_call_base_c(SyntaxNodeType_e::TNativeFunctionCall) {}

  const std::string& name() const override { return id->name(); }

  void debugPrint(const size_t tab = 1,
                  std::function<size_t()> onOutPrefix = nullptr) const override {
    _PRINT_WORD_PREFIX(children.empty());
    id->printInfo();
    SyntaxNode_c::debugPrint(tab, onOutPrefix);
  }

  _GEN_VALUE(WordItem_nativeCall_c, id);
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
