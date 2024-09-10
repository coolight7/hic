#pragma once

// 虚拟机

#include <cstdint>

class HicVMCore_c {
public:
  static constexpr int stackSize = 1024 * 8;

  HicVMCore_c() {}

  // 通用寄存器
  int64_t ax = 0, bx = 0, cx = 0, dx = 0;
  char* pc = nullptr;
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