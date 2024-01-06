#pragma once

#include <fstream>
#include <queue>
#include <string>
#include <regex>

#include <json/json.hpp>
#include <spdlog/spdlog.h>

#include "processor/common.h"
#include "utils/shell_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

namespace svc {

class Compiler {
public:
    // Get code version
    static std::string GetCodeVersion(const std::string& source_code) {
        std::regex pattern("pragma solidity");
        std::vector<std::string> line_data = StringUtils::SplitString(source_code, '\n');
        std::string result = "";
        for (auto &line : line_data) {
            if (std::regex_search(line, pattern)) {
                std::string version;
                bool flag = false;
                for (auto &c : line) {
                    if (flag) {
                        if (c == '.' || (c >= '0' && c <= '9')) {
                            version += c;
                        } else {
                            break;
                        }
                    } else if (c == '0') {
                        version += c;
                        flag = true;
                    }
                }
                result = HigherVersion(result, version);
            }
        }
        if (kSupportedVersions.find(result) != kSupportedVersions.end()) {
            return result;
        }
        return "";
    };

    // Switch solidity version
    static int SwitchVersion(const std::string &version) {
        std::string _version;
        // 一个大版本统一使用最高的小版本, 比如0.5.x使用0.5.17
        if (version == "4") _version = "0.4.25";
        if (version == "5") _version = "0.5.17";
        if (version == "6") _version = "0.6.12";
        if (ShellUtils::RunCommand("solc-select use " + _version)) {
           return SWITCH_VERSION_ERROR;
        }
        return SUCCESS;
    }

    static int CleanSourceCode(std::string& source_code) {
        StringUtils::RemoveNonUTF8(source_code);
        StringUtils::RemoveCommentsInFile(source_code);
        StringUtils::RemoveEmptyLine(source_code);
        StringUtils::ProcessEndOfLine(source_code);
        // add license for source code
        source_code = "// SPDX-License-Identifier: MIT\n" + source_code;
        spdlog::debug({"Pre-remove successfully."});
        return SUCCESS;
    }

    // Compile source code to json ast
    static int CompileToJson(std::string& source_code, nlohmann::json& ast_json) {
        CleanSourceCode(source_code);
        // write source code to tmp file
        FileUtils::WriteStringToFile(kTmpFile, source_code);
        // compile source code
        int ret = ShellUtils::RunCommand("solc --ast-json --overwrite " + kTmpFile + " -o " + kTmpPath);
        if (ret) {
            return COMPILE_ERROR;
        }
        std::ifstream ast_json_file(kTmpJsonFile);
        ast_json = nlohmann::json::parse(ast_json_file);
        if (ast_json.empty()) {
            return AST_JSON_ERROR;
        }
        return SUCCESS;
    }

    // Get nodes from json AST tree
    static std::vector<nlohmann::json> GetNodesByName(nlohmann::json& root, const std::string& key, const std::string& value) {
        std::vector<nlohmann::json> nodes;
        std::queue<nlohmann::json> q;
        q.push(root);
        while (!q.empty()) {
            nlohmann::json node = q.front();
            q.pop();
            for (auto& iter : node.items()) {
                if (iter.key() == key && iter.value().is_string() && iter.value() == value) {
                    nodes.push_back(node);
                }
                if (iter.key() == "children" && iter.value().is_array()) {
                    for (auto& child : iter.value()) {
                        q.push(child);
                    }
                }
            }
        }
        return nodes;
    }

    static std::string HigherVersion(std::string& v1, std::string& v2) {
        std::vector<std::string> v1_vec = StringUtils::SplitString(v1, '.');
        std::vector<std::string> v2_vec = StringUtils::SplitString(v2, '.');
        if (v1 == "" || v1_vec.size() != 3) return v2;
        if (v2 == "" || v2_vec.size() != 3) return v1;
        for (int i = 0; i < 3; i++) {
            int a = std::stoi(v1_vec[i]);
            int b = std::stoi(v2_vec[i]);
            if (a == b) continue;
            if (a < b) return v2;
            if (a > b) return v1;
        }
        return v1;
    }
};

} // namespace svc
