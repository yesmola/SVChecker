#include "token.h"

namespace solver {

void Token::Set(std::string type, std::string literal, int64_t line) {
    type_ = type;
    literal_ = literal;
    line_ = line; 
}

std::string Token::GetType() {
    return type_;
}

std::string Token::GetLiteral() {
    return literal_;
}

int64_t Token::GetLine() {
    return line_;
}

void Token::ReplaceType(std::string type) {
    type_ = type;
}

void Token::ReplaceLiteral(std::string literal) {
    literal_ = literal;
}

void Token::ReplaceLine(int64_t line) {
    line_ = line;
}

} // namespace solver