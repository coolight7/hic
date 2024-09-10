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

  char* data = nullptr;
  char* code = nullptr;
};

/**
 * 段：
 * - .STACK
 * - .DATA
 * - .CODE
 */