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

#define GENERATE_FUN_ITEM_d(...)                                                                   \
  _NameTagConcat_d(_GENERATE_FUN_ITEM, _MacroArgToTag_d(__VA_ARGS__))(__VA_ARGS__)
#define _GENERATE_FUN_ITEM() GENERATE_FUN_ITEM_d
#define _GENERATE_FUN_ITEM1(fun)                                                                   \
  std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<WordItem_c>)>(                       \
      std::bind(&SyntacticAnalysis_c::fun, this, std::placeholders::_1))
#define _GENERATE_FUN_ITEMN(fun, ...)                                                              \
  _GENERATE_FUN_ITEM1(fun), _MacroDefer_d(_GENERATE_FUN_ITEM)()(__VA_ARGS__)

#define _REBACK_d(word_ptr, tempIndex, ...)                                                        \
  reback_funs(word_ptr, tempIndex, _MoreExpand_d(GENERATE_FUN_ITEM_d(__VA_ARGS__)))

// 语法树节点
class SyntaxNode_c : public ListNode_c {
public:
  template <typename... _Args>
  inline static std::shared_ptr<SyntaxNode_c> make_node(std::shared_ptr<_Args>... args) {
    auto re_ptr = std::make_shared<SyntaxNode_c>();
    re_ptr->children.push_back(args...);
    return re_ptr;
  }

  SyntaxNode_c() : ListNode_c(ListNodeType_e::Syntactic) {}

  const std::string& name() const override {
    if (nullptr != node) {
      return node->name();
    }
    return HicUtil_c::emptyString;
  }

  bool set(std::shared_ptr<WordItem_c> ptr) {
    assert(nullptr == node);
    if (nullptr == ptr) {
      node = ptr;
      return true;
    }
    return false;
  }

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

  void debugPrint(const size_t tab = 1, std::function<size_t()> onOutPrefix = nullptr) const {
    int index = 0;
    for (const auto& item : children) {
      switch (item->nodeType) {
      case ListNodeType_e::Lexical: {
        // word
        if (tab > 0) {
          size_t prefixTab = 0;
          if (nullptr != onOutPrefix) {
            prefixTab = onOutPrefix();
          }
          for (int i = tab - prefixTab - 1; i-- > 0;) {
            std::cout << "   ";
          }
        }
        if (children.size() == index + 1) {
          // end
          std::cout << "  └──";
        } else {
          std::cout << "  ├──";
        }
        item->printInfo();
      } break;
      case ListNodeType_e::Syntactic: {
        // Syntax
        bool isEnd = (index == children.size() - 1);

        size_t prefixTab = 0;
        if (nullptr != onOutPrefix) {
          prefixTab = onOutPrefix();
        }
        for (int i = tab - prefixTab - 1; i-- > 0;) {
          std::cout << "   ";
        }
        if (isEnd) {
          std::cout << "  └──┐";
        } else {
          std::cout << "  ├──┐";
        }
        SyntaxNode_c* item_ptr = (SyntaxNode_c*)item.get();
        if (nullptr != item_ptr->node) {
          // 节点包含内容
          std::cout << "# " << item_ptr->node->name();
        }
        std::cout << std::endl;
        item_ptr->debugPrint(tab + 1, [&onOutPrefix, isEnd]() -> size_t {
          size_t size = 0;
          if (nullptr != onOutPrefix) {
            size = onOutPrefix();
          }
          if (false == isEnd) {
            // 补充
            std::cout << "  │";
          } else {
            std::cout << "   ";
          }
          return size + 1;
        });
      } break;
      default:
        break;
      }
      index++;
    }
  }

  std::shared_ptr<WordItem_c> node = nullptr;
  std::list<std::shared_ptr<ListNode_c>> children{};
};

class SyntacticAnalysis_c {
public:
  inline static bool enableLog_assertToken = false;
  inline static bool enableLog_parseCode = true;

  void init(std::string_view in_code) { lexicalAnalyse.init(in_code); }

  /**
   * ## 回溯依次调用
   */
  template <typename T>
  std::shared_ptr<SyntaxNode_c>
  reback(std::shared_ptr<T>&& word_ptr, int tempIndex,
         std::function<std::shared_ptr<SyntaxNode_c>(std::shared_ptr<T>)>&& fun) {
    lexicalAnalyse.tokenIndex = tempIndex;
    return fun(word_ptr);
  }

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

  std::shared_ptr<SyntaxNode_c> parse_value_define(std::shared_ptr<WordItem_c> word_ptr) {
    // value_type
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(parse_value_type(word_ptr))) {
      // 指针或引用
      _GEN_WORD_DEF(next);
      const auto sign = assertToken_type(next_ptr, WordEnumToken_e::Tsign);
      if (nullptr != sign) {
        if (sign->name() != "*" && sign->name() != "&") {
          return nullptr;
        }
        re_node->add(sign);
        return re_node;
      }
      // 下一个 [token] 不是符号，回退一个
      lexicalAnalyse.tokenBack();
      return re_node;
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_value_define_id(std::shared_ptr<WordItem_c> word_ptr) {
    // value_type
    auto re_node = parse_value_define(word_ptr);
    if (nullptr != re_node) {
      // ID
      if (re_node->add(assertToken_type(nullptr, WordEnumToken_e::Tid))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_value_set(std::shared_ptr<WordItem_c> word_ptr) {
    auto pre_node = std::make_shared<SyntaxNode_c>();
    if (pre_node->add(assertToken_type(word_ptr, WordEnumToken_e::Tid))) {
      auto re_node = SyntaxNode_c::make_node(pre_node);
      if (re_node->add(assertToken_sign(nullptr, "="))) {
        if (re_node->add(parse_expr(nullptr, WordEnumType_e::Tvoid))) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_value_define_init(std::shared_ptr<WordItem_c> word_ptr) {
    auto pre_node = parse_value_define_id(word_ptr);
    if (pre_node) {
      auto re_node = SyntaxNode_c::make_node(pre_node);
      if (re_node->add(assertToken_sign(nullptr, "="))) {
        if (re_node->add(parse_expr(nullptr, WordEnumType_e::Tvoid))) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_expr(std::shared_ptr<WordItem_c> word_ptr,
                                           WordEnumType_e ret_type) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    std::shared_ptr<WordItem_c> last_ptr;
    int group = 0;
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
        }
        if (doRet) {
          lexicalAnalyse.tokenBack();
          return re_node;
        }
      }
      re_node->add(word_ptr);
      last_ptr = word_ptr;
      word_ptr = nullptr;
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_expr_void(std::shared_ptr<WordItem_c> word_ptr) {
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

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_return(std::shared_ptr<WordItem_c> word_ptr,
                                                       WordEnumType_e ret_type) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Treturn}))) {
      if (re_node->add(parse_expr(nullptr, ret_type))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_return_void(std::shared_ptr<WordItem_c> word_ptr) {
    return parse_code_ctrl_return(word_ptr, WordEnumType_e::Tvoid);
  }

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_if(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tif})) &&
        re_node->add(assertToken_sign(nullptr, "(")) &&
        re_node->add(parse_expr(nullptr, WordEnumType_e::Tbool)) &&
        re_node->add(assertToken_sign(nullptr, ")")) &&
        re_node->add(assertToken_sign(nullptr, "{"))) {
      // if (expr) { code }
      _GEN_WORD_DEF(sign);
      if (re_node->add(assertToken_sign(sign_ptr, "}"))) {
        // 空代码块 {}
        return re_node;
      }
      if (re_node->add(parse_code(sign_ptr)) && re_node->add(assertToken_sign(nullptr, "}"))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_while(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Twhile})) &&
        re_node->add(assertToken_sign(nullptr, "(")) &&
        re_node->add(parse_expr(nullptr, WordEnumType_e::Tbool)) &&
        re_node->add(assertToken_sign(nullptr, ")")) &&
        re_node->add(assertToken_sign(nullptr, "{"))) {
      // while (expr) { code }
      _GEN_WORD_DEF(sign);
      if (re_node->add(assertToken_sign(sign_ptr, "}"))) {
        // 空代码块 {}
        return re_node;
      }
      if (re_node->add(parse_code(nullptr)) && re_node->add(assertToken_sign(nullptr, "}"))) {
        return re_node;
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_code_ctrl_for(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken(word_ptr, WordItem_ctrl_c{WordEnumCtrl_e::Tfor}))) {
      if (re_node->add(assertToken_sign(nullptr, "(")) && re_node->add(parse_expr_void(nullptr)) &&
          re_node->add(parse_expr(nullptr, WordEnumType_e ::Tbool)) &&
          re_node->add(parse_expr_void(nullptr)) && re_node->add(assertToken_sign(nullptr, ")"))) {
        // for (expr;expr;expr) { code }
        if (re_node->add(assertToken_sign(nullptr, "{"))) {
          _GEN_WORD_DEF(sign);
          if (re_node->add(assertToken_sign(sign_ptr, "}"))) {
            // 空代码块 {}
            return re_node;
          }
          if (re_node->add(parse_code(nullptr)) && re_node->add(assertToken_sign(nullptr, "}"))) {
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
      result = _REBACK_d(word_ptr, tempIndex, parse_value_define_init, parse_value_set,
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
  std::shared_ptr<SyntaxNode_c> parse_function_call(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken_type(word_ptr, WordEnumToken_e::Tid))) {
      if (re_node->add(assertToken_sign(nullptr, "("))) {
        std::shared_ptr<WordItem_c> sign_ptr;
        // 参数列表
        while (re_node->add(assertToken_type(nullptr, WordEnumToken_e::Tid))) {
          _GEN_WORD(sign);
          if (false == re_node->add(assertToken_sign(sign_ptr, ","))) {
            break;
          }
        }
        if (re_node->add(assertToken_sign(sign_ptr, ")"))) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_function_define(std::shared_ptr<WordItem_c> word_ptr) {
    // {返回值} {函数名} ({参数列表}*) { {code} }
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(parse_value_define(word_ptr))) {
      // 返回值
      if (re_node->add(assertToken_type(nullptr, WordEnumToken_e::Tid))) {
        // 函数名
        if (re_node->add(assertToken_sign(nullptr, "("))) {
          std::shared_ptr<WordItem_c> sign_ptr;
          // 参数列表
          _GEN_WORD(sign);
          while (re_node->add(parse_value_define_id(sign_ptr))) {
            _GEN_WORD(sign);
            if (false == re_node->add(assertToken_sign(sign_ptr, ","))) {
              break;
            }
          }
          // code
          if (re_node->add(assertToken_sign(sign_ptr, ")")) &&
              re_node->add(assertToken_sign(nullptr, "{")) && re_node->add(parse_code(nullptr)) &&
              re_node->add(assertToken_sign(nullptr, "}"))) {
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
      if (re_node->add(assertToken_sign(nullptr, "("))) {
        std::shared_ptr<WordItem_c> sign_ptr;
        while (re_node->add(parse_value_define_id(nullptr))) {
          _GEN_WORD(sign);
          if (false == re_node->add(assertToken_sign(sign_ptr, ","))) {
            break;
          }
        }
        if (re_node->add(assertToken_sign(sign_ptr, ")")) &&
            re_node->add(assertToken_sign(nullptr, "{")) && re_node->add(parse_code(nullptr)) &&
            re_node->add(assertToken_sign(nullptr, "}"))) {
          return re_node;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_c> parse_enum_define(std::shared_ptr<WordItem_c> word_ptr) {
    auto re_node = std::make_shared<SyntaxNode_c>();
    if (re_node->add(assertToken(word_ptr, WordItem_type_c{WordEnumType_e::Tenum}))) {
      if (re_node->add(assertToken_type(nullptr, WordEnumToken_e::Tid)) &&
          re_node->add(assertToken_sign(nullptr, "{"))) {
        // ID 列表
        std::shared_ptr<WordItem_c> sign_ptr;
        // TODO: 解析 id <= number>?
        while (re_node->add(assertToken_type(nullptr, WordEnumToken_e::Tid))) {
          _GEN_WORD(sign);
          if (false == re_node->add(assertToken_sign(sign_ptr, ","))) {
            break;
          }
        }
        if (re_node->add(assertToken_sign(sign_ptr, "}"))) {
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

  SyntaxNode_c root = SyntaxNode_c(false);
  LexicalAnalyse_c lexicalAnalyse{};
};