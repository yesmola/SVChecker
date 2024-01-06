#pragma once

#include <string>
#include <vector>
#include <memory>

#include "lexer/base.h"
#include "lexer/lexer.h"
#include "lexer/token.h"

namespace svc {

class Parser {
public:
    explicit Parser(std::string source_code) {
        lexer_ = std::make_shared<Lexer>(source_code);
        // Read two tokens, so cur_token_ and peek_token_ are both set
        GetNextToken();
        GetNextToken();
    }
    ~Parser() = default;

    void ParseProgram() {
        while (!CurTokenIs(FEOF)) {
            token_flow_.push_back(cur_token_->GetLiteral());
            GetNextToken();
        }
    }

    std::vector<std::shared_ptr<Token>> GetTokenFlow() {
        return lexer_->GetTokenFlow();
    }
private:
    void GetNextToken() {
        cur_token_ = peek_token_;
        peek_token_ = lexer_->GetNextToken();
    }
    bool CurTokenIs(std::string t) { return cur_token_->GetType() == t; }
    bool PeekTokenIs(std::string t) { return peek_token_->GetType() == t; }

private:
    std::shared_ptr<Lexer> lexer_;
    std::shared_ptr<Token> cur_token_;
    std::shared_ptr<Token> peek_token_;
    std::vector<std::string> token_flow_;
};

} // namespace svc