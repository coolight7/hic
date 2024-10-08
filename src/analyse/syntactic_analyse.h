#pragma once

// 语法分析

#include <functional>
#include <list>
#include <memory>
#include <stack>

#include "entity.h"
#include "lexical_analyse.h"

/**
 * - 注意此处也需要 `##__VA_ARGS__`，否则会多传 `,逗号` 给 UtilLog导致展开异常
 */
#define SynLineLog(level, tip, ...)                                                                \
  UtilLineLog(level,                                                                               \
              "[" + WordEnumToken_c::toName(lexicalAnalyse.currentToken()->token) + "] " +         \
                  lexicalAnalyse.currentToken()->name(),                                           \
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

#include "src/magic/macro.h"
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

class SyntacticAnalysis_c {
public:
  inline static bool enableLog_assertToken = false;
  inline static bool enableLog_parseCode = true;

  bool init(std::string_view in_code) {
    root = std::make_shared<SyntaxNode_group_c>();
    return lexicalAnalyse.init(in_code);
  }

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
      SynLineLog(Twarning, "[assertToken] faild; word: {}, expect: {}", word.formatInfo(),
                 limit.formatInfo());
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

  std::shared_ptr<WordItem_id_c> assertToken_id(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    _GEN_WORD(word);
    if (WordEnumToken_e::Tid == word.token) {
      return HicUtil_c::toType<WordItem_id_c>(word_ptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_nativeCall_c>
  assertToken_nativeCall(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    _GEN_WORD(word);
    if (WordEnumToken_e::TnativeCall == word.token) {
      return HicUtil_c::toType<WordItem_nativeCall_c>(word_ptr);
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_operator_c>
  assertToken_sign(WordEnumOperator_e sign, std::shared_ptr<WordItem_c> word_ptr = nullptr,
                   bool startWith = false) {
    return HicUtil_c::toType<WordItem_operator_c>(
        assertToken(WordItem_operator_c{sign}, word_ptr, startWith));
  }

  std::shared_ptr<WordItem_operator_c>
  assertToken_sign(const std::string_view sign, std::shared_ptr<WordItem_c> word_ptr = nullptr,
                   bool startWith = false) {
    return HicUtil_c::toType<WordItem_operator_c>(
        assertToken(WordItem_operator_c{WordItem_operator_c::toEnum(sign)}, word_ptr, startWith));
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

  std::shared_ptr<WordItem_string_c> parse_constexpr_string(std::shared_ptr<WordItem_c> word_ptr) {
    auto result = assertToken_type(WordEnumToken_e::Tstring, word_ptr);
    if (nullptr != result) {
      return HicUtil_c::toType<WordItem_string_c>(result);
    }
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_value_define_c>
  parse_value_define(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::make_shared<SyntaxNode_value_define_c>();
    auto type_limit = TypeLimit_e::Normal;
    {
      // 类型限制符号
      _GEN_WORD(word);
      auto preType = assertToken_type(WordEnumToken_e::Tkeyword, word_ptr);
      if (nullptr != preType) {
        auto& key = preType->toKeyword();
        switch (key.value) {
        case WordEnumCtrl_e::Tconst: {
          type_limit = TypeLimit_e::ValueConstexpr;
        } break;
        case WordEnumCtrl_e::Tfinal: {
          type_limit = TypeLimit_e::Final;
        } break;
        default:
          return nullptr;
        }
        word_ptr = nullptr;
      }
    }
    // 类型符号
    _GEN_WORD(word);
    if (WordEnumToken_e::Ttype == word.token || WordEnumToken_e::Tid == word.token) {
      auto use_type = WordEnumType_e::Tvoid;
      if (WordEnumToken_e::Ttype == word.token) {
        use_type = word.toType().value;
      }
      auto& value_type = re_node->make_value_type(use_type);
      value_type->limit = type_limit;
      value_type->word = word_ptr;
      // 指针或引用
      while (true) {
        _GEN_WORD_DEF(next);
        auto sign_ptr = assertToken_type(WordEnumToken_e::Toperator, next_ptr);
        if (nullptr != sign_ptr) {
          auto& sign = sign_ptr->toOperator();
          if (sign.value == WordEnumOperator_e::TMulti) {
            (value_type->pointer)++;
          } else if (sign.value == WordEnumOperator_e::TBitAnd) {
            value_type->isReferer = true;
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
      if (re_node->set_id(HicUtil_c::toType<WordItem_id_c>(assertToken_id()))) {
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
        switch (word.token) {
        case WordEnumToken_e::Tid: {
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
        } break;
        case WordEnumToken_e::Toperator:
        case WordEnumToken_e::Tnumber:
        case WordEnumToken_e::Tstring:
        case WordEnumToken_e::Tvalue:
        case WordEnumToken_e::TnativeCall: {
          // none
        } break;
        case WordEnumToken_e::Tundefined:
        case WordEnumToken_e::Tkeyword:
        case WordEnumToken_e::Ttype:
        default:
          return nullptr;
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
          // TODO: 支持设置 parse_code 的返回值是否添加 符号范围限制
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
      SynLineLog(Tdebug, "parse_code -----vvv------ ");
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
          SynLineLog(Tdebug, "  - parse_code/line --vvv-- ");
        }
        result->debugPrint(2);
      }
      word_ptr = nullptr;
    } while (nullptr != result);
    if (enableLog_parseCode) {
      SynLineLog(Tdebug, "parse_code -----^^^----- ");
    }
    return re_node;
  }

  std::shared_ptr<SyntaxNode_c>
  parse_function_call(std::shared_ptr<WordItem_c> word_ptr = nullptr) {
    auto re_node = std::shared_ptr<SyntaxNode_c>{};
    const auto name_id = assertToken_id(word_ptr);
    if (nullptr != name_id) {
      // {ID_userFun} ()
      auto node = std::make_shared<SyntaxNode_function_call_c>();
      node->set_id(name_id);
      re_node = node;
    } else {
      // {ID_nativeFun} ()
      const auto name_native = assertToken_nativeCall();
      if (nullptr != name_native) {
        auto node = std::make_shared<SyntaxNode_native_call_c>();
        node->set_id(name_native);
        re_node = node;
      }
    }
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
    return nullptr;
  }

  std::shared_ptr<SyntaxNode_function_define_base_c>
  parse_function_define(std::shared_ptr<WordItem_c> word_ptr) {
    const auto return_type = parse_value_define(word_ptr);
    // 返回值
    if (nullptr != return_type) {
      // 函数名
      const auto name_id = assertToken_id();
      if (nullptr != name_id) {
        // {返回值} {函数名} ({参数列表}*) { {code} }
        auto re_node = std::make_shared<SyntaxNode_function_define_user_c>();
        re_node->set_return_type(return_type);
        re_node->set_id(name_id);
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
          if (assertToken_sign(")", sign_ptr) && assertToken_sign("{") &&
              re_node->set_body(parse_code()) && assertToken_sign("}")) {
            // code
            // TODO: 支持设置 parse_code 的返回值是否添加 符号范围限制
            return re_node;
          }
        }
      } else {
        // {返回值} {Native函数名} ({参数列表}*);
        const auto name_nativeCall = assertToken_nativeCall();
        if (nullptr != name_nativeCall) {
          auto re_node = std::make_shared<SyntaxNode_function_define_native_c>();
          re_node->set_return_type(return_type);
          re_node->set_id(name_nativeCall);
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
            if (assertToken_sign(")", sign_ptr) && assertToken_sign(";")) {
              return re_node;
            }
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
    if (re_node->add(assertToken_id(word_ptr))) {
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
      if (re_node->set_id(assertToken_id()) && assertToken_sign("{")) {
        // ID 列表
        std::shared_ptr<WordItem_c> sign_ptr;
        // TODO: 解析 id = int?
        int index = 0;
        while (true) {
          _GEN_WORD(sign);
          auto item = assertToken_id(sign_ptr);
          if (nullptr == item) {
            break;
          }
          auto result = std::make_shared<SyntaxNode_value_define_init_c>();
          result->set_define_id(std::make_shared<SyntaxNode_value_define_id_c>());
          result->define_id->id = item;
          result->define_id->set_value_define(std::make_shared<SyntaxNode_value_define_c>());
          result->define_id->value_define->make_value_type(WordEnumType_e::Tint,
                                                           TypeLimit_e::ValueConstexpr, nullptr);
          result->set_data(std::make_shared<SyntaxNode_operator_c>(WordEnumOperator_e::TNone));
          auto data = std::make_shared<WordItem_number_c>(index);
          result->data->add(data);

          re_node->add(result);
          sign_ptr = nullptr;
          {
            _GEN_WORD(sign);
            if (nullptr == assertToken_sign(",", sign_ptr)) {
              break;
            }
          }
          sign_ptr = nullptr;
          index++;
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
    Assert_d(nullptr != root, "根节点 root 不应为 nullptr");
    UtilLog(Tinfo, "语法分析：");
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
    }
    return true;
  }

  // 语法分析结果，抽象语法树根节点
  std::shared_ptr<SyntaxNode_group_c> root;
  // 词法分析器
  LexicalAnalyse_c lexicalAnalyse{};
};

#undef SynLineLog
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

#include "src/magic/unset_macro.h"