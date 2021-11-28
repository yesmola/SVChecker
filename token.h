#pragma once

#include <string>
#include <unordered_map>

namespace solver {

class Token {
  public:
    Token(){};
    Token(std::string type, std::string literal, int64_t line) {
      type_ = type;
      literal_ = literal;
      line_ = line;
    }
    ~Token(){};
    void Set(std::string type, std::string literal, int64_t line = 0);
    std::string GetType();
    std::string GetLiteral();
    int64_t GetLine();
    void ReplaceType(std::string type);
    void ReplaceLiteral(std::string literal);
    void ReplaceLine(int64_t line);

  private:
    std::string type_;
    std::string literal_;
    int64_t line_;
};

} // namespace solver
