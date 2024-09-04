#pragma once

#include <array>
#include <string>

#include "magic/macro.h"

/**
 * 关键字（控制）
 */
GENERATE_ENUM(WordEnumCtrl, const, static, if, else, switch, case, default, break, continue, while, do, for, return)

/**
 * 关键字（变量类型）
 */
GENERATE_ENUM(WordEnumType, void, char, bool, int, int64, float, float64, pointer, enum, class,
              function)

/** token 分类
 * Tundefined,
 * Tnumber,     // 数值
 * Tstring,     // 字符串
 * Tsign,       // 符号
 * Tkeyword,    // 关键字
 * Ttype,       // 类型
 * TnativeCall, // 内置函数
 * Tid,         // 名称
 */
GENERATE_ENUM(WordEnumToken, undefined, number, string, sign, keyword, type, nativeCall, id)

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
 *  - QueryDot       // ?.
 * - 2
 *  - Not            // !expr
 *  - Shift          // ~expr
 *  - StartAddAdd    // ++expr
 *  - StartSubSub    // --expr
 * - 3
 *  - Multi          // *
 *  - Division       // /
 *  - Semicolon      // %
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
 *  - SetMulti       // *=
 *  - SetDivision    // /=
 *  - SetAdd         // +=
 *  - SetSub         // -=
 *  - SetNullMerge   // ??=
 */
GENERATE_ENUM(WordEnumExpr, EndAddAdd, EndSubSub, CurvesGroup, SquareGroup, Dot, QueryDot, Not,
              Shift, StartAddAdd, StartSubSub, Multi, Division, Semicolon, Add, Sub, BitLeftMove,
              BitRightMove, BitAnd, BitOr, GreaterOrEqual, Greater, LessOrEqual, Less, Equal,
              NotEqual, And, Or, NullMerge, IfElse, Set, SetMulti, SetDivision, SetAdd, SetSub,
              SetNullMerge)

#include "magic/unset_macro.h"