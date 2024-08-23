#pragma once

#include <array>
#include <string>

#include "magic/macro.h"

/**
 * 关键字（控制）
 */
GENERATE_ENUM(WordValueCtrl, const, static, if, else, switch, case, default, break, while, do, for, return)

/**
 * 关键字（变量类型）
 */
GENERATE_ENUM(WordValueType, void, char, bool, int, int64, float, float64, pointer, class, function)

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
GENERATE_ENUM(WordValueToken, undefined, number, string, sign, keyword, type, nativeCall, id)

/** 内置函数
 *
 */
GENERATE_ENUM(WordValueNativeCall, print, sizeof, malloc, free, exit, main)

#include "magic/unset_macro.h"