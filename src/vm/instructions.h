#pragma once
// 自定义指令集

#include <array>
#include <string>

#include "src/magic/macro.h"

GENERATE_ENUM(
    Instruction,
    // 值复制
    // 只能用在两个寄存器或立即数之间;
    // - mov {目的寄存器} {源寄存器}
    // - mov {目的寄存器} {立即数}
    MOV,
    // 计算地址值后写入寄存器
    // - LEA ax, [bx + 4]
    LEA,
    LC, // load  char - load Rd, [Rn, #offset]  ;从内存 $Rn + offset 加载数据到寄存器 Rd
    LI, // load  int
    LL, // load  long(int64)
    SC, // store char - store Rd, [Rn, #offset] ;将寄存器 Rd 中的数据存储到内存 $Rn + offset
    SI,   // store int
    SL,   // store long(int64)
    ADD,  // +
    SUB,  // -
    MUL,  // *
    DIV,  // /
    MOD,  // 求余
    SHL,  // 左移 <<
    SHR,  // 右移 >>
    NOT,  // 取反
    XOR,  // 异或
    AND,  // &&
    OR,   // ||
    EQ,   // 相等 ==
    NEQ,  // 不相等 !=
    LT,   // 小于 <
    GT,   // 大于 >
    LE,   // 小于等于 <=
    GE,   // 大于等于 >=
    PUSH, // push {源寄存器}
    POP,  // pop {目的寄存器}
    CALL, // call {function}
    RET,  // return
    NVAR, // 创建新栈帧
    DARG, // 删除当前栈帧
    JMP,  // jump
    JZ,   // jump 如果结果为0或相等
    JNZ,  // jump 如果结果非0或不等
    NCALL // 内置函数调用
);

GENERATE_ENUM(RegisterId,
              AX, // 通用寄存器
              BX, CX, DX,
              CS, // pc
              IP,
              EBP, // stack
              ESP);

#include "src/magic/unset_macro.h"

class VMConfig_c {
public:
  VMConfig_c() = delete;

  // 指令长度（byte）
  static constexpr int instructionSize = 1;
  // 单个寄存器的大小（byte）
  static constexpr int registerSize = 8;
  // 地址长度（byte）
  static constexpr int addressSize = 8;

  static Instruction_e getStore(int size) {
    assert(size > 0 && size <= 8);
    if (size > 4) {
      return Instruction_e::TSL;
    } else if (size > 1) {
      return Instruction_e::TSI;
    }
    return Instruction_e::TSC;
  }

  static Instruction_e getLoad(int size) {
    assert(size > 0 && size <= 8);
    if (size > 4) {
      return Instruction_e::TLL;
    } else if (size > 1) {
      return Instruction_e::TLI;
    }
    return Instruction_e::TLC;
  }
};
