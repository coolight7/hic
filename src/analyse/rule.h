#pragma once

#include <array>
#include <map>
#include <string>

#include "src/magic/macro.h"

/**
 * 关键字（控制）
 */
GENERATE_ENUM(WordEnumCtrl, const, final, static, if, else, switch, case, default, break, continue, while, do, for, return)

/**
 * 关键字（变量类型）
 */
GENERATE_ENUM(WordEnumType, void, char, bool, int, int64, float, float64, enum, class, function)

/** token 分类
 * Tundefined,
 * Tnumber,     // 数值
 * Tstring,     // 字符串
 * Toperator    // 操作符
 * Tkeyword,    // 关键字
 * Ttype,       // 类型
 * TnativeCall, // 内置函数
 * Tid,         // 名称
 */
GENERATE_ENUM(WordEnumToken, undefined, number, string, operator, keyword, type, nativeCall, id)

/** 内置函数
 *
 */
GENERATE_ENUM(WordEnumNativeCall, print, sizeof, malloc, free, exit)

/**
 * ## 符号优先级
 * - 1
 *  - EndAddAdd      // expr++
 *  - EndSubSub      // expr--
 *  - CurvesGroup    // ()
 *  - SquareGroup    // []
 *  - Dot            // .
 *  - NullDot        // ?.
 * - 2
 *  - Not            // !expr
 *  - Shift          // ~expr
 *  - StartAddAdd    // ++expr
 *  - StartSubSub    // --expr
 * - 3
 *  - Multi          // *
 *  - Division       // /
 *  - Percent        // %
 * - 4
 *  - Add           // +
 *  - Sub           // -
 * - 5
 *  - BitLeftMove   // <<
 *  - BitRightMove  // >>
 * - 6
 *  - BitAnd        // &
 * - 7
 *  - BitOr         // |
 * - 8
 *  - GreaterOrEqual // >=
 *  - Greater        // >
 *  - LessOrEqual    // <=
 *  - Less           // <
 * - 9
 *  - Equal          // ==
 *  - NotEqual       // !=
 * - 10
 *  - And            // &&
 * - 11
 *  - Or             // ||
 * - 12
 *  - NullMerge      // ??
 * - 13
 *  - IfElse         // expr ? if_body : else_body
 * - 14
 *  - Set            // =
 *  - SetBitOr       // |=
 *  - SetBitAnd      // &=
 *  - SetMulti       // *=
 *  - SetDivision    // /=
 *  - SetAdd         // +=
 *  - SetSub         // -=
 *  - SetNullMerge   // ??=
 *
 * ##
 *  - FlowerGroup    // {}
 *  - Semicolon      // ;
 *  - Comma          // ,
 */
GENERATE_ENUM(WordEnumOperator, Undefined, None, EndAddAdd, EndSubSub, LeftCurvesGroup,
              RightCurvesGroup, LeftSquareGroup, RightSquareGroup, Dot, NullDot, Level1, Not, Shift,
              StartAddAdd, StartSubSub, Level2, Multi, Division, Percent, Add, Sub, BitLeftMove,
              BitRightMove, BitAnd, BitOr, GreaterOrEqual, Greater, LessOrEqual, Less, Equal,
              NotEqual, And, Or, NullMerge, IfElse, Set, SetBitOr, SetBitAnd, SetMulti, SetDivision,
              SetAdd, SetSub, SetNullMerge, LeftFlowerGroup, RightFlowerGroup, Semicolon, Comma,
              END)

/**
 * 符号类型
 */
GENERATE_ENUM(SymbolType, Value, Function, Enum, Class);

#include "src/magic/unset_macro.h"
#include "src/util.h"

class WordItem_c;
class WordItem_id_c;
class SyntaxNode_c;
class SyntaxNode_value_define_c;
class SymbolItem_c;
class SymbolItem_function_c;
class SymbolItem_value_c;

using SymbolTable = std::map<std::string, std::shared_ptr<SymbolItem_c>>;

enum ListNodeType_e {
  Lexical,
  Syntactic,
  Symbol,
};

class ListNode_c {
public:
  ListNode_c(ListNodeType_e in_type) : nodeType(in_type) {}
  ListNode_c(const ListNode_c&) = delete;

  virtual void printInfo() const {}
  virtual const std::string& name() const { return HicUtil_c::emptyString; }
  virtual std::shared_ptr<SyntaxNode_value_define_c> returnType() const { return nullptr; }

  ListNodeType_e nodeType;
};
