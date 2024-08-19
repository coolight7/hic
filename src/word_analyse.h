#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

/**
 * 变量类型
 */
enum WordValueType_e {
  Tchar,
  Tint,
  Tpointer,
};

enum WordValueClass_e { Cnumber, CFunction, Csyscall, C };

class WordValue_c {
public:
  WordValueClass_e valueClass;
  WordValueType_e valueType;
  long long value;
};

class WordItem_c {
public:
  std::string hash() {}

  std::string token;
  std::string name;
  // 作用域变量声明
  std::vector<WordValue_c> valuelist;
};

class WordAnalyse_c {
public:
  inline static const std::string_view reserveKeywords[]{
      // 类型
      "void",
      "char",
      "bool",
      "int",
      "long",
      "struct",
      "class",
      // 范围
      "const",
      "static",
      // 控制
      "if",
      "else",
      "while",
      "do",
      "for",
      "return",
      // fun
      "print",
      "sizeof",
  };

  inline static std::string_view isReserve(std::string_view it_ptr) {
    for (const auto item : reserveKeywords) {
      if (it_ptr.starts_with(item)) {
        return item;
      }
    }
    return std::string_view{};
  }

  // <isSuccess, str>
  std::pair<bool, std::string> analyse(std::string_view code) {
    symbolTable.clear();
    int line = 1;
    for (auto it_ptr = code.data(); it_ptr != code.end();) {
      const auto it = *it_ptr;
      ++it_ptr;
      if ('\n' == it) {
        ++line;
        continue;
      }
      if ('\r' == it) {
        continue;
      }
      if ('#' == it) {
        // 换行到下一行
        while ('\n' != *it_ptr && code.end() != it_ptr) {
          ++it_ptr;
        }
        ++line;
        continue;
      }
      {
        const bool isLowaz = (it >= 'a' && it <= 'z');
        if (isLowaz || (it >= 'A' && it <= 'Z') || '_' == it) {
          if (isLowaz) {
            const auto result = isReserve(std::string_view{it_ptr, code.end() - it_ptr});
            if (false == result.empty()) {
              // 是关键字
              it_ptr += result.size();
              symbolTable[""];
              continue;
            }
          }
        }
      }
      if (it >= '0' && it <= '9') {
      }
    }
  }

  void debugPrint() {
    for (const auto& it : symbolTable) {
      std::cout << it.first << ": " << it.second.name << "\t" << it.second.token << std::endl;
    }
  }

  std::map<std::string, WordItem_c> symbolTable;
};