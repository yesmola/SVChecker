#include <regex>

#include "base.h"

namespace solver {

// find keyword
std::string Util::LookUpIdent(std::string literal) {
    if (std::regex_match(literal, std::regex("bytes\\d{0,}"))) return BYTES;
   /* if (std::regex_match(literal, std::regex("fixed\\d{0,}x\\d{0,}"))) return FIXED;
    if (std::regex_match(literal, std::regex("ufixed\\d{0,}x\\d{0,"))) return UFIXED;*/
    auto iter = keywords.find(literal);
    if (iter != keywords.end()) {
        return iter->second;
    }
    return IDENT;
}

// check virable define
bool Util::IsVirableDefine(std::string literal) {
    if (std::regex_match(literal, std::regex("bytes\\d{0,}"))) return true;
    if (std::regex_match(literal, std::regex("int\\d{0,}"))) return true;
    if (std::regex_match(literal, std::regex("uint\\d{0,}"))) return true;
   /* if (std::regex_match(literal, std::regex("fixed\\d{0,}x\\d{0,}"))) return true;
    if (std::regex_match(literal, std::regex("ufixed\\d{0,}x\\d{0,"))) return true;*/
    return variable_define.find(literal) != variable_define.end();
}

// check API
bool Util::IsAPI(std::string type) {
    return api_set.find(type) != api_set.end();
}

// check Letter
bool Util::IsLetter(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z';
}

// check digit
bool Util::IsDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

// read file into a string
std::string Util::ReadFileIntoString(std::string file_name) {
    std::ifstream open_file(file_name);
    std::ostringstream buf;
    char ch;
    while (buf && open_file.get(ch)) {
        buf.put(ch);
    }
    return buf.str();
}

} // namespace solver