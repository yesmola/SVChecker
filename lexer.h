#pragma once

#include <string>
#include <vector>

#include "token.h"

namespace solver {

class Lexer {
  public:
    Lexer() {
        input_ = "";
        input_length_ = input_.length();
        position_ = 0;
        read_position_ = 0;
        line_num_ = 1;
        token_flow_.clear();
        ReadChar();
    }
    Lexer(std::string input) : input_(input) {
        input_length_ = input_.length();
        position_ = 0;
        read_position_ = 0;
        line_num_ = 1;
        token_flow_.clear();
        ReadChar();
    }
    ~Lexer() {}

    Token* GetNextToken();
    int64_t GetLineNum();

  private:
    void ReadChar();
    char PeekChar();
    void SkipWhitespace();
    std::string ReadIdentifier();
    std::string ReadNumber();

  private:
    std::string input_;
    int64_t input_length_;
    int64_t position_;
    int64_t read_position_;
    int64_t line_num_;
    char ch_;
    std::vector<Token*> token_flow_;
};

} // namespace solver