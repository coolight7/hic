## Hi-C
- 计划包含内容：
    - 类c语言的编译器前端，但生成自定义的汇编指令
        - 类c源码-> 抽象语法树 -> ASM
    - 汇编解释器
- 效果应当类似于动态语言的jit解释执行器

## 计划内容
- 编译生成汇编：
    - 词法分析（√）
    - 语法分析（√）
    - 语义分析（...）
    - 汇编生成
- 汇编解释器：
    - 寄存器与内存分段（√）
    - 指令集设计（...）
    - VM/指令执行