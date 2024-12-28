#pragma once

// 虚拟机

#include <cstdint>

#include "../analyse/generate_asm.h"
#include "instructions.h"

class HicVMCore_c {
public:
  static constexpr int stackSize = 1024 * 8;

  template <typename T> T readCodeItem(size_t& i) {
    if constexpr (std::is_same_v<T, Instruction_e>) {
      auto result = Instruction_c::toEnum(*(InstructionByte_t*)(program->code.c_str() + i));
      i += sizeof(InstructionByte_t);
      std::cout << Instruction_c::toName(result) << "\t";
      return result;
    } else if constexpr (std::is_same_v<T, RegisterId_e>) {
      auto result = RegisterId_c::toEnum(*(RegisterByte_t*)(program->code.c_str() + i));
      i += sizeof(RegisterByte_t);
      std::cout << RegisterId_c::toName(result) << "\t";
      return result;
    } else {
      auto result = *(T*)(program->code.c_str() + i);
      i += sizeof(T);
      std::cout << std::format("s{}(0x{:X})", sizeof(T), result) << "\t";
      return result;
    }
  }

public:
  HicVMCore_c() {}


  void run(std::shared_ptr<ProgramPackage_c> in_program) {
    program = in_program;
    std::cout << std::endl << "vvv ----------- <code> ----------- vvv" << std::endl;
    bool start = true;
    const char* ptr = program->code.c_str();
    int op_num_size = 0;
    int op_num_type_size = 0;
    for (size_t i = 0; i < program->code.size();) {
      if (ptr[i] == '\n') {
        ++i;
        start = true;
        std::cout << std::endl;
        continue;
      }
      if (start) {
        auto item = Instruction_c::toEnum(readCodeItem<Instruction_e>(i));
        start = false;
        switch (item) {
        case Instruction_e::TMOVR: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TMOVI: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<Immediate64_t>(i);
        } break;
        case Instruction_e::TLEA: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate64_t>(i);
        } break;
        case Instruction_e::TLC: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate8_t>(i);
        } break;
        case Instruction_e::TLS: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate16_t>(i);
        } break;
        case Instruction_e::TLI: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate32_t>(i);
        } break;
        case Instruction_e::TLL: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate64_t>(i);
        } break;
        case Instruction_e::TSC: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate8_t>(i);
        } break;
        case Instruction_e::TSS: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate16_t>(i);
        } break;
        case Instruction_e::TSI: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate32_t>(i);
        } break;
        case Instruction_e::TSL: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
          auto arg3 = readCodeItem<Immediate64_t>(i);
        } break;
        case Instruction_e::TADD: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TSUB: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TMUL: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TDIV: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TMOD: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TSHL: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TSHR: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TNOT: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TXOR: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TAND: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TOR: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TEQ: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TNEQ: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TLT: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TGT: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TLE: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TGE: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TPUSH: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TPOP: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TCALL: {
          auto arg1 = readCodeItem<Immediate64_t>(i);
        } break;
        case Instruction_e::TRET: {
        } break;
        case Instruction_e::TNVAR: {
        } break;
        case Instruction_e::TDARG: {
        } break;
        case Instruction_e::TJMP: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TJZ: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TJNZ: {
          auto arg1 = readCodeItem<RegisterId_e>(i);
          auto arg2 = readCodeItem<RegisterId_e>(i);
        } break;
        case Instruction_e::TNCALL: {
          auto arg1 = readCodeItem<Immediate64_t>(i);
        } break;
        default:
          break;
        }
      } else {
        UtilLog(Twarning, "unknown instruction: char({})", readCodeItem<unsigned char>(i));
      }
    }
    std::cout << std::endl << "^^^ ----------- <code> ----------- ^^^" << std::endl;
  }

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

  protected:
  std::shared_ptr<ProgramPackage_c> program = nullptr;
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