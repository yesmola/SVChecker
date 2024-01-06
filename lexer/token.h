#pragma once

#include <string>
#include <unordered_map>

namespace svc {

class Token {
  public:
    Token() = default;
    Token(std::string type, std::string literal, int64_t line) {
      type_ = type;
      literal_ = literal;
      line_ = line;
    }
    ~Token() = default;

    void Set(std::string type, std::string literal, int64_t line = 0) {
        type_ = type;
        literal_ = literal;
        line_ = line; 
    }
    std::string GetType() { return type_; }
    std::string GetLiteral() { return literal_; }
    int64_t GetLine() { return line_; }
    
    void ReplaceType(std::string type) { type_ = type; }
    void ReplaceLiteral(std::string literal) { literal_ = literal; }
    void ReplaceLine(int64_t line) { line_ = line; }

  private:
    std::string type_;
    std::string literal_;
    int64_t line_;
};

} // namespace svc