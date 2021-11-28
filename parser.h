#pragma once

#include <stack>
#include <set>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <fstream>

#include "base.h"
#include "lexer.h"
#include "token.h"

namespace solver {

class Parser {
  public:
    explicit Parser(Lexer *lexer) {
        lexer_ = lexer;
        total_contract_num_ = 0;
        total_function_num_ = 0;
        // Read twoo tokens, so cur_token_ and peek_token_ are bot set
        GetNextToken();
        GetNextToken();
    }
    ~Parser() {}    

  public:
    void ParseProgram();
    void PrintTokenFlow();

  private:
    // helper
    void GetNextToken();
    bool CurTokenIs(std::string t);
    bool PeekTokenIs(std::string t);
    void SkipComment();
    void Clear();
    // workflow
    void ParseContract();
    void ParseLibrary();
    void ParseGlobalVar();
    void ParseFunction();
    void ParseEvent();
    void ParseModifier();
    void ParseConstructor();
    void GetCodePatch();

  public:
    int64_t total_contract_num_;
    int64_t total_function_num_;
    std::vector<std::vector<Token*>> code_patches;

  private:
    Lexer *lexer_;
    Token* cur_token_;
    Token* peek_token_;
    std::set<std::string> struct_enum_names_; // 记录结构体和枚举变量的名字 还有合约的名字 合约也可以用来定义变量
    std::unordered_map<std::string, std::vector<Token*>> global_var_tokens_; // 记录全局变量包含的 token
    std::unordered_map<std::string, std::vector<Token*>> function_tokens_; // 记录函数包含的 token
};

} // namespace solver