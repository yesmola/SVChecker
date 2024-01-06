#pragma once

#include <string>
#include <vector>
#include <memory>

#include "lexer/token.h"
#include "lexer/base.h"

namespace svc {

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
    ~Lexer() = default;

    std::shared_ptr<Token> GetNextToken() {
        token_flow_.push_back(std::make_shared<Token>());
        std::shared_ptr<Token> tok = token_flow_[token_flow_.size() - 1];
        SkipWhitespace();
        switch (ch_) {
        case '=':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(EQ, "==", line_num_);
            } else if (PeekChar() == '>') {
                ReadChar();
                tok->Set(ARROW, "=>", line_num_);
            } else {
                tok->Set(ASSIGN, std::string(1, ch_), line_num_);
            }
            break;
        case '+':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(PLUSEQ, "+=", line_num_);
            } else {
                tok->Set(PLUS, std::string(1, ch_), line_num_);
            }
            break;
        case '%':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(MODEQ, "%=", line_num_);
            } else {
                tok->Set(MOD, std::string(1, ch_), line_num_);
            }
            break;
        case '-':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(MINUSEQ, "-=", line_num_);
            } else {
                tok->Set(MINUS, std::string(1, ch_), line_num_);
            }
            break;
        case '/':
            if (PeekChar() == '/') {
                ReadChar();
                tok->Set(CCOMMENT, "//", line_num_);
            } else if (PeekChar() == '*') {
                ReadChar();
                tok->Set(LPCOMMENT, "/*", line_num_);
            } else {
                tok->Set(SPLASH, std::string(1, ch_), line_num_);
            }
            break;
        case '*':
            if (PeekChar() == '/') {
                ReadChar();
                tok->Set(RPCOMMENT, "*/", line_num_);
            } else {
                tok->Set(ASTERISK, std::string(1, ch_), line_num_);
            }
            break;
        case '!':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(NEQ, "!=", line_num_);
            } else {
                tok->Set(BANG, std::string(1, ch_), line_num_);
            }
            break;
        case '<':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(LTOEQ, "<=", line_num_);
            } else {
                tok->Set(LT, std::string(1, ch_), line_num_);
            }
            break;
        case '>':
            if (PeekChar() == '=') {
                ReadChar();
                tok->Set(GTOEQ, ">=", line_num_);
            } else {
                tok->Set(GT, std::string(1, ch_), line_num_);
            }
            break;
        case '(':
            tok->Set(LPAREN, std::string(1, ch_), line_num_);
            break;
        case ')':
            tok->Set(RPAREN, std::string(1, ch_), line_num_);
            break;
        case '[':
            tok->Set(LSBRACE, std::string(1, ch_), line_num_);
            break;
        case ']':
            tok->Set(RSBRACE, std::string(1, ch_), line_num_);
            break;
        case '{':
            tok->Set(LBRACE, std::string(1, ch_), line_num_);
            break;
        case '}':
            tok->Set(RBRACE, std::string(1, ch_), line_num_);
            break;
        case '.':
            tok->Set(POINT, std::string(1, ch_), line_num_);
            break;
        case ',':
            tok->Set(COMMA, std::string(1, ch_), line_num_);
            break;
        case ';':
            tok->Set(SEMICOLON, std::string(1, ch_), line_num_);
            break;
        case '&':
            if (PeekChar() == '&') {
                ReadChar();
                tok->Set(AND, "&&", line_num_);
            } else {
                tok->Set(BAND, std::string(1, ch_), line_num_);
            }
            break;
        case '|':
            if (PeekChar() == '|') {
                ReadChar();
                tok->Set(OR, "||", line_num_);
            } else {
                tok->Set(BOR, std::string(1, ch_), line_num_);
            }
            break;
        case '\'':
            tok->Set(SQUO, std::string(1, ch_), line_num_);
            break;
        case '\"':
            tok->Set(DQUO, std::string(1, ch_), line_num_);
            break;
        case '\\':
            ReadChar();
            tok->Set(RSPLASH, "translation character", line_num_);
            break;
        case 0:
            tok->Set(FEOF, "", line_num_);
            break;
        default:
            if (std::isalpha(ch_) || ch_ == '_') {
                std::string literal = ReadIdentifier();
                tok->Set(helper::LookUpIdent(literal), literal, line_num_);
                return tok;
            } else if (std::isdigit(ch_)) {
                std::string literal = ReadNumber();
                tok->Set(NUMB, literal, line_num_);
                return tok;
            } else {
                tok->Set(ILLEGAL, std::string(1, ch_), line_num_);
            }
            break;
        }

        ReadChar();
        return tok;
    }

    int64_t GetLineNum() { return line_num_; }

    std::vector<std::shared_ptr<Token>> GetTokenFlow() {
        return token_flow_;
    }

  private:
    void ReadChar() {
        if (read_position_ >= input_length_) {
            ch_ = 0;
        } else {
            ch_ = input_[read_position_];
            position_ = read_position_;
            read_position_++;
        }
    }
    char PeekChar() {
        if (read_position_ >= input_length_) {
            return 0;
        } else {
            return input_[read_position_];
        }
    }

    void SkipWhitespace() {
        while (ch_ == ' ' || ch_ == '\t' || ch_ == '\n' || ch_ == '\r') {
            if (ch_ == '\n')
                line_num_++;
            ReadChar();
        }
    }
    std::string ReadIdentifier() {
        std::string ident;
        while (std::isalpha(ch_) || std::isdigit(ch_) || ch_ == '_') {
            ident += ch_;
            ReadChar();
        }
        return ident;
    }
    std::string ReadNumber()  {
        std::string num;
        while (std::isalpha(ch_) || std::isdigit(ch_) || ch_ == '.') {
            num += ch_;
            ReadChar();
        }
        return num;
    }

  private:
    std::string input_;
    int64_t input_length_;
    int64_t position_;
    int64_t read_position_;
    int64_t line_num_;
    char ch_;
    std::vector<std::shared_ptr<Token>> token_flow_;
};

} // namespace svc