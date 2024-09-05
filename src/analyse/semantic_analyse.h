#pragma once

// 语义分析

#include "syntactic_analyse.h"

class SemanticAnalyse_c {
public:
  void init(std::string_view in_code) {
    // 添加 global 全局符号表
    symbolTablePush();
    syntacticAnalysis.init(in_code);
  }

  bool analyse() {
    auto syntactic_result = syntacticAnalysis.analyse();
    if (false == syntactic_result) {
      return false;
    }
    // 语义分析

    return true;
  }

  std::map<const std::string, std::shared_ptr<WordItem_c>>& globalSymbolTable() {
    Assert_d(symbolTable.empty() == false);
    return symbolTable.front();
  }

  std::map<const std::string, std::shared_ptr<WordItem_c>>& currentSymbolTable() {
    Assert_d(symbolTable.empty() == false);
    return symbolTable.back();
  }

  void symbolTablePush() {
    symbolTable.emplace_back(std::map<const std::string, std::shared_ptr<WordItem_c>>{});
  }

  void symbolTablePop() { symbolTable.pop_back(); }
  // 在 [symbolTable] 中查找符号，且是从最近/最小的作用域开始查找
  std::shared_ptr<WordItem_c> symbolTableFind(const std::string& key) {
    if (false == symbolTable.empty()) {
      for (auto it = symbolTable.end() - 1;; --it) {
        if (it->contains(key)) {
          return (*it)[key];
        }
        if (it == symbolTable.begin()) {
          break;
        }
      }
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> globalSymbolTableFind(const std::string& key) {
    auto& curr = globalSymbolTable();
    if (curr.contains(key)) {
      return curr[key];
    }
    return nullptr;
  }

  std::shared_ptr<WordItem_c> currentSymbolTableFind(const std::string& key) {
    auto& curr = currentSymbolTable();
    if (curr.contains(key)) {
      return curr[key];
    }
    return nullptr;
  }

  void debugPrintSymbolTable(bool printKeyword) {
    int i = 0;
    for (const auto& table : symbolTable) {
      std::cout << i + 1 << std::endl;
      for (const auto& it : table) {
        if (false == printKeyword && (it.second->token == WordEnumToken_e::Tkeyword ||
                                      it.second->token == WordEnumToken_e::Ttype ||
                                      it.second->token == WordEnumToken_e::TnativeCall)) {
          continue;
        }
        for (auto j = i; j-- > 0;) {
          std::cout << "  ";
        }
        std::cout << "- ";
        std::cout << it.first << ":    \t" << it.second->name() << "\t"
                  << WordEnumToken_c::toName(it.second->token) << std::endl;
      }
    }
  }

  // 作用域符号表
  std::vector<std::map<const std::string, std::shared_ptr<WordItem_c>>> symbolTable{};
  SyntacticAnalysis_c syntacticAnalysis{};
};