#pragma once

class HicCore_c {
public:
  static constexpr int stackSize = 1024 * 8;

  // 通用寄存器
  long long ax, bx;
  char* pc;
  char* ebp;
  char* esp;
  // 栈
  char stack[stackSize]{};
  char* data;
  char* code;
};

/**
 * 段：
 * - .STACK
 * - .DATA
 * - .CODE
 */