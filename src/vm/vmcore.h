#pragma once

// 虚拟机

#include <cstdint>

#include "instructions.h"

class HicVMCore_c {
public:
  static constexpr int stackSize = 1024 * 8;

  HicVMCore_c() {}

  // 通用寄存器
  int64_t ax = 0, bx = 0, cx = 0, dx = 0;
  // pc
  char* cs = nullptr; // 代码段寄存器
  char* ip = nullptr;
  // stack pointer
  char* ebp = nullptr;
  char* esp = nullptr;
  // 栈
  char stack[stackSize]{};

  char* heap = nullptr;
  char* data = nullptr;
  char* code = nullptr;
};

/**
 * ## 段：
 * - .STACK
 * - .DATA
 * - .CODE
 *
 * ## 约定
 * - 函数调用：
 *  - R0 作为函数返回值
 *  - R1, R2, R3 作为函数参数
 *  - 父函数调用子函数前，需要将 R0-R3 都压入栈保存，子函数可随意使用寄存器，然后
 * 返回之后由父函数使用和恢复。
 *  - 父函数分配子函数堆栈
 */