#pragma once
// 自定义指令集

#include <array>
#include <string>

#include "src/magic/macro.h"

GENERATE_ENUM(Instruction,
              // 值复制
              // 只能用在两个寄存器或立即数之间;
              // - mov {目的寄存器} {源寄存器}
              // - mov {目的寄存器} {立即数}
              MOV,
              // 计算地址值后写入寄存器
              // - LEA ax, [bx + 4]
              LEA,
              LC,         // load char
              LI,         // load int
              LL,         // load long(int64)
              SC,         // save char
              SI,         // save int
              SL,         // save long
              ADD,        // +
              SUB,        // -
              MUL,        // *
              DIV,        // /
              MOD,        // 求余
              SHL,        // 左移 <<
              SHR,        // 右移 >>
              NOT,        // 取反
              XOR,        // 异或
              AND,        // &&
              OR,         // ||
              EQ,         // 相等 ==
              NE,         // 不相等 !=
              LT,         // 小于 <
              GT,         // 大于 >
              LE,         // 小于等于 <=
              GE,         // 大于等于 >=
              PUSH,       // push {源寄存器}
              POP,        // pop {目的寄存器}
              CALL,       // call {function}
              RET,        // return
              NVAR,       // 创建新栈帧
              DARG,       // 删除当前栈帧
              JMP,        // jump
              JZ,         // jump 如果结果为0或相等
              JNZ,        // jump 如果结果非0或不等
              NativeCall, // 内置函数调用
);

#include "src/magic/unset_macro.h"

class VMConfig_c {
public:
  // 指令长度（bit）
  static constexpr int instructionSize = 8;
  // 单个寄存器的大小（bit）
  static constexpr int registerSize = 64;
  // 地址长度（bit）
  static constexpr int addressSize = 64;
};
