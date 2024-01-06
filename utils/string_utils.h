#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <regex>

namespace svc {

class StringUtils {
public:
    static std::string StrToUpper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    static std::string StrToLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    static std::vector<std::string> SplitString(const std::string &s, char delim) {
        std::string temp;
        std::stringstream string_stream { s };
        std::vector<std::string> result;

        while (std::getline(string_stream, temp, delim)) {
            result.push_back(temp);
        }

        return result;
    }

    static std::string RemoveBlockComments(std::string& str) {
        bool insideComment = false;
        std::string result = "";

        for (size_t i = 0; i < str.length(); i++) {
            if (!insideComment && i < str.length() - 1 && str[i] == '/' && str[i + 1] == '*') {
                insideComment = true;
                i++;
            } else if (insideComment && i < str.length() - 1 && str[i] == '*' && str[i + 1] == '/') {
                insideComment = false;
                i++;
            } else if (!insideComment && i < str.length() - 1 && str[i] == '/' && str[i + 1] == '/') {
                // Ignore single-line comments
                break;
            } else if (!insideComment) {
                result += str[i];
            }
        }
        return result;
    }

    // Remove non-UTF8 characters from the string.
    static void RemoveNonUTF8(std::string& str) {
        str.erase(std::remove_if(str.begin(), str.end(), [](char c) {
            return (c & 0xC0) == 0x80;
        }), str.end());
    }

    // 清除代码中多余的空行
    static void RemoveEmptyLine(std::string& str) {
        bool pre_line_is_empty = false;
        std::string result;
        for (char c : str) {
            if (c == '\n') {
                if (!pre_line_is_empty) {
                    result += c;
                }
                pre_line_is_empty = true;
            } else {
                result += c;
                pre_line_is_empty = false;
            }
        }
        str = result;
        return;
    }

    // 整理源代码，使每一行以'{' ';' '}'结尾
    static void ProcessEndOfLine(std::string& str) {
        std::string output;
        int st = 0; // 记录括号()
        for (size_t i = 0; i < str.size(); i++) {
            char c = str[i];
            output += c;
            if (c == '(') {
                st ++;
            }
            if (c == ')') {
                st --;
            }
            if ((st == 0 && c == ';') || c == '{' || c == '}') {
                size_t k = i + 1;
                while (k < str.size() && str[k] == ' ') {
                    k ++;
                }
                if (k < str.size() && str[k] != '\n') {
                    output += '\n';
                }
            }
        }
        str = output;
        return;
    }
    // Remove comments from the string.
    static void RemoveCommentsInFile(std::string& str) {
        // 排除代码中的 https:// 或者 http://
        std::regex url_regex("https://|http://");
        str = std::regex_replace(str, url_regex, "");
        std::regex line_comment_regex("//.*");
        str = std::regex_replace(str, line_comment_regex, "");
        str = StringUtils::RemoveBlockComments(str);
        return;
    }
};

} // namespace svc