#pragma once

#include <json/json.hpp>

#include <algorithm>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <set>
#include <iostream>
#include <chrono>
#include <sstream>

#include "processor/common.h"
#include "processor/compiler.h"
#include "utils/file_utils.h"
#include "utils/shell_utils.h"
#include "utils/string_utils.h"
#include "lexer/lexer.h"
#include "lexer/parser.h"
#include "ast/ast_wrapper.h"

namespace svc {

class Processor {
public:
    Processor() {
        gen_num_ = 0;
        secure_num_ = 0;
        max_gen_num_ = INT_MAX;
        max_secure_num_ = INT_MAX;
    }

    Processor(int n) {
        gen_num_ = 0;
        secure_num_ = 0;
        max_gen_num_ = n;
        max_secure_num_ = n;
    }

    struct FuncInfo {
        FuncInfo() {
            func_node = nullptr;
            code = "";
            line = "";
            token_flow = "";
        }
        FuncInfo(nlohmann::json f, std::string l, std::string func_code) {
            func_node = f;
            code = func_code;
            line = l;
            token_flow = "";
        } 

        FuncInfo(nlohmann::json f, std::string l, std::string func_code, std::set<std::string>& origin_set) {
            func_node = f;
            code = func_code;
            line = l;
            token_flow = "";
            detect_token = origin_set;
        } 

        nlohmann::json func_node;
        std::string code;
        std::string line;
        std::set<std::string> detect_token; 
        std::string token_flow;
    };

    virtual ~Processor() = default;

    virtual void Init(const std::string& file, std::string& source_code, nlohmann::json& ast_json, const std::string& dst_path) {
        file_ = StringUtils::SplitString(file, '.')[0];
        source_code_ = source_code;
        ast_json_ = ast_json;
        dst_path_ = dst_path + GetName() + "/";
        inject_info_.clear();
        secure_info_.clear();
    }

    virtual void Init(const std::string& file, std::shared_ptr<AstWrapper> ast, const std::string& dst_path) {
        file_ = StringUtils::SplitString(file, '.')[0];
        source_code_ = ast->GetSourceCode();
        ast_ = ast;
        ast_json_ = ast->GetJsonNode();
        dst_path_ = dst_path + GetName() + "/";
        inject_info_.clear();
        secure_info_.clear();
    }

    int BuildVulnerbaleDataset() {
        int ret = SUCCESS;
        ret = ExtractInject();
        ret = InjectSourceCode();
        return ret;
    }

    int BuildDataset() {
        int ret = SUCCESS;
        // 1. 提取代码段
        ret = ExtractInject();
        if (ret) { return ret; }
        // 2. 植入漏洞
        ret = Inject();
        if (ret) { return ret; }
        // 3. 拓展func, 加入所有外部变量定义
        ExpandFuncNode();
        //
        Eliminate();
        // 4. 标准化token流
        GenCodeSnippets();
        // 5. 落数据
        DumpCodeSnippets();
        return ret;
    }

    virtual std::string GetName() = 0;

    int GetGenNum() { return gen_num_; }
    int GetSecureNum() { return secure_num_; }
    bool Finish() { return vulnerable_counter_ >= max_gen_num_ && secure_counter_ >= max_secure_num_; }
    int GetRepeatNum() { return repeat_snippet_; }

protected:
    virtual int Extract() { return SUCCESS; }
    virtual int ExtractInject() { return SUCCESS; }
    virtual int ExtractCanditate() { return SUCCESS; }
    virtual int Inject() { return SUCCESS; }
    virtual int InjectSourceCode() { 
        Inject();

        spdlog::debug("[{}] Dump Source code...", GetName());
        for (auto& info : inject_info_) {
            std::string source_code = source_code_;
            int start = GetStartPosAndLen(info.func_node["src"]).first;
            int len = GetStartPosAndLen(info.func_node["src"]).second;
            source_code.replace(start, len, info.code);
            FileUtils::CreateDir(dst_path_);
            std::ofstream f(dst_path_ + file_ + "_" + std::to_string(vulnerable_counter_) + ".sol");
            vulnerable_counter_++;
            secure_counter_++;
            f << source_code;
            f.flush();
            f.close();
        }
        return SUCCESS; 
    }

    virtual void ExpandFuncNode() {
        spdlog::debug("[{}] Start expanding...", GetName());
        
        std::unordered_map<int, std::string> id_src_map = ast_->GetAllIdSrc();
        auto expand = [&] (std::vector<FuncInfo>& func_info) {
            for (FuncInfo& info : func_info) {
                std::string code = "";
                nlohmann::json root = info.func_node;
                std::queue<nlohmann::json> q;
                std::set<int> visit_id;
                q.push(root);
                while (!q.empty()) {
                    nlohmann::json cur = q.front();
                    q.pop();
                    
                    if (cur.contains("attributes") && cur.at("attributes").contains("referencedDeclaration") &&
                        cur["attributes"]["referencedDeclaration"] != nullptr) {
                        int ref_id = cur["attributes"]["referencedDeclaration"];
                        if (id_src_map.find(ref_id) != id_src_map.end() && visit_id.find(ref_id) == visit_id.end()) {
                            std::string define_str = source_code_.substr(GetStartPosAndLen(id_src_map[ref_id]).first,
                                GetStartPosAndLen(id_src_map[ref_id]).second);
                            if (define_str.back() != ';') {
                                define_str += ";";
                            }
                            while (define_str.find("\n") != define_str.npos) {
                                define_str.replace(define_str.find("\n"), 1, " ");
                            }
                            code += define_str + std::string("\n");
                            visit_id.insert(ref_id);
                        }
                    }
                    for (auto& iter : cur.items()) {
                        if (iter.key() == "children" && iter.value().is_array()) {
                            for (auto& child : iter.value()) {
                                q.push(child);
                            }
                        }
                    }
                }
                // spdlog::debug("{}", info.code);
                // spdlog::debug("**********************");
                info.code = code + info.code;
                // spdlog::debug("{}", info.code);
                // spdlog::debug("---------------------------------");
            }
        };
        
        expand(secure_info_);
        expand(inject_info_);
        return;
    }

    virtual void Eliminate() {
        spdlog::debug("[{}] Start eliminate...", GetName());
        auto eliminate = [&] (std::vector<FuncInfo>& func_info) {
            for (auto& info : func_info) {
                if (info.detect_token.empty()) {
                    continue;
                }
                auto line_part = StringUtils::SplitString(info.code, '\n');
                std::vector<int> need(line_part.size(), 0);
                for (size_t i = 0; i < line_part.size(); i++) {
                    if (!need[i]) {
                        bool flag = false;
                        for (auto t : info.detect_token) {
                            if (line_part[i].find(t) != line_part[i].npos) {
                                flag = true;
                                need[i] = 1;
                                break;
                            }
                        }
                        if (flag) {
                            if (line_part[i].back() == '{') {
                                int st = 1;
                                for (size_t j = i + 1; j < line_part.size(); j++) {
                                    if (line_part[j].back() == '{') {
                                        st ++;
                                    }
                                    if (line_part[j].back() == '}') {
                                        st --;
                                    }
                                    if (st == 0) {
                                        need[j] = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                std::string code = "";
                for (size_t i = 0; i < line_part.size(); i++) {
                    if (need[i]) {
                        code += line_part[i] + "\n";
                    }
                }
                // spdlog::debug("{}", info.code);
                // spdlog::debug("**********************");
                info.code = code;
                // spdlog::debug("{}", info.code);
                // spdlog::debug("---------------------------------");
            }
        };

        spdlog::debug("[{}] Start eliminate secure snippets...", GetName());
        eliminate(secure_info_);
        spdlog::debug("[{}] Start eliminate vulnerble snippets...", GetName());
        eliminate(inject_info_);
        return;
    }

    virtual void GenCodeSnippets() {
        spdlog::debug("[{}] Start gen code snippets...", GetName());
        // todo(yuanye)
        std::unordered_map<std::string, DefType> all_def_map = GetDefinitionMap(ast_json_, false);
        std::set<std::string> all_num = ast_->GetAllNumber();
    
        auto gen_code_snippet = [&] (std::vector<FuncInfo>& func_info) {
            for (auto& info : func_info) {
                std::unordered_map<std::string, DefType> func_def_map = GetDefinitionMap(info.func_node);
                std::shared_ptr<Parser> parser = std::make_shared<Parser>(info.code);
                parser->ParseProgram();

                std::map<DefType, std::unordered_map<std::string, std::string>> replace_map = { {DefType::CONTRACT_DEF, {}},
                                                                                                {DefType::FUNC_DEF, {}},
                                                                                                {DefType::STRUCT_DEF, {}},
                                                                                                {DefType::VAR_DEF, {}},};
                
                auto token_flow = parser->GetTokenFlow();
                for (size_t i = 0; i < token_flow.size(); ++i) {
                    auto token = token_flow[i];
                    std::string word = token->GetLiteral();
                    // 保护一下tx.origin add sub
                    if (word == "tx" || word == "origin" || word == "add" || word =="sub") {
                        continue;
                    }
                    // call.value
                    if (word == "value" && i > 1 && token_flow[i - 2]->GetLiteral() == "call") {
                        continue;
                    }
                    if (all_num.find(word) != all_num.end()) {
                        token->ReplaceLiteral("NUM");
                    }
                    DefType type = DefType::UNKNOWN;
                    if (func_def_map.find(word) != func_def_map.end()) {
                        type = func_def_map.at(word);
                    } else if (all_def_map.find(word) != all_def_map.end()) {
                        type = all_def_map.at(word);
                    }
                    if (type != DefType::UNKNOWN) {
                        if (replace_map.at(type).find(word) != replace_map.at(type).end()) {
                            token->ReplaceLiteral(replace_map[type][word]);
                        } else {
                            if (type == DefType::CONTRACT_DEF) {
                                replace_map[type][word] = "CONTRACT" + std::to_string(replace_map[type].size() + 1);
                            } else if (type == DefType::FUNC_DEF) {
                                replace_map[type][word] = "FUNC" + std::to_string(replace_map[type].size() + 1);
                            } else if (type == DefType::STRUCT_DEF) {
                                replace_map[type][word] = "STRUCT" + std::to_string(replace_map[type].size() + 1);
                            } else if (type == DefType::VAR_DEF) {
                                replace_map[type][word] = "VAR" + std::to_string(replace_map[type].size() + 1);
                            }
                            token->ReplaceLiteral(replace_map[type][word]);
                        }
                    }
                }
                std::string snippet;
                for (auto& token : parser->GetTokenFlow()) {
                    snippet += token->GetLiteral() + " ";
                }
                info.token_flow = snippet;
                spdlog::debug("{}", snippet);
                spdlog::debug("{}", info.code);
                spdlog::debug("---------------------------------");
            }
        };

        spdlog::debug("[{}] Start gen secure snippets...", GetName());
        gen_code_snippet(secure_info_);
        spdlog::debug("[{}] Start gen vulnerble snippets...", GetName());
        gen_code_snippet(inject_info_);
        return;
    }

    virtual void DumpCodeSnippets() {
        spdlog::debug("[{}] Start dump code snippets...", GetName());
        
        std::string dst_file_name = GetTimeForFileName();

        auto dump_code_snippet = [&] (std::vector<FuncInfo>& func_info, bool has_bug) {
            int k = 0;
            for (auto& info : func_info) {
                std::string snippet = info.token_flow;
                if (token_flow_set_.find(snippet) == token_flow_set_.end()) {
                    token_flow_set_.insert(snippet);
                } else {
                    repeat_snippet_ ++;
                    continue;
                }
                k++;
                std::string output = dst_file_name + "_" + std::to_string(k);
                if (has_bug) {
                    FileUtils::CreateDir(dst_path_ +  "vulnerable/");
                    std::ofstream f(dst_path_ + "vulnerable/" + output + ".txt");
                    FileUtils::CreateDir(dst_path_ +  "vulnerable_source/");
                    std::ofstream f2(dst_path_ + "vulnerable_source/" + output + ".txt");
                    vulnerable_counter_ ++;
                    f << snippet << std::endl;
                    f2 << info.line << std::endl;
                    f2 << "----------" << std::endl;
                    f2 << info.code << std::endl;
                    f.flush();
                    f.close();
                    f2.flush();
                    f2.close();
                } else {
                    FileUtils::CreateDir(dst_path_ +  "secure/");
                    std::ofstream f(dst_path_ + "secure/" + output + ".txt");
                    FileUtils::CreateDir(dst_path_ +  "secure_source/");
                    std::ofstream f2(dst_path_ + "secure_source/" + output + ".txt");
                    secure_counter_ ++;
                    f << snippet << std::endl;
                    f2 << info.line << std::endl;
                    f2 << "----------" << std::endl;
                    f2 << info.code << std::endl;
                    f.flush();
                    f.close();
                    f2.flush();
                    f2.close();
                }
            }
        };
        
        dump_code_snippet(secure_info_, false);
        dump_code_snippet(inject_info_, true);
        return;
    }

    bool IsUint(const std::string& type) {
        return type == "uint" || type == "uint8" || type == "uint16" || type == "uint32" || 
            type == "uint64" || type == "uint128" || type == "uint256";
    }

    // get all "name" : "Assignment" nodes
    static std::vector<nlohmann::json> GetAssignmentNodes(nlohmann::json& root) {
        return Compiler::GetNodesByName(root, "name", "Assignment");
    }

    // get all "name" : "FunctionDefinition" nodes
    static std::vector<nlohmann::json> GetFunctionDefinitionNodes(nlohmann::json& root) {
        return Compiler::GetNodesByName(root, "name", "FunctionDefinition");
    }

    // get all "name" : "ContractDefinition" nodes
    static std::vector<nlohmann::json> GetContractDefinitionNodes(nlohmann::json& root) {
        return Compiler::GetNodesByName(root, "name", "ContractDefinition");
    }

    // get all "name" : "VariableDeclarationStatement" nodes
    static std::vector<nlohmann::json> GetVarStatementNodes(nlohmann::json& root) {
        return Compiler::GetNodesByName(root, "name", "VariableDeclarationStatement");
    }

    // get all "name" : "ExpressionStatement" nodes
    static std::vector<nlohmann::json> GetExpStatementNodes(nlohmann::json& root) {
        return Compiler::GetNodesByName(root, "name", "ExpressionStatement");
    }

    // src : "start_pos:len:0", get start position and length
    std::pair<int, int> GetStartPosAndLen(std::string src) {
        std::vector<std::string> info = StringUtils::SplitString(src, ':');
        int start_pos = std::stoi(info[0]);
        int length = std::stoi(info[1]);
        return std::make_pair(start_pos,  length);
    }

    // find node in which func, return this func
    std::string GetFuncSourceCode(nlohmann::json& node, std::vector<nlohmann::json>& func_nodes) {
        auto [start_pos, len] = GetStartPosAndLen(node["src"].get<std::string>());
        for (auto& func_node : func_nodes) {
            auto [func_start_pos, func_len] = GetStartPosAndLen(func_node["src"].get<std::string>());
            if (start_pos >= func_start_pos && start_pos + len <= func_start_pos + func_len) {
                return source_code_.substr(func_start_pos, func_len);
            }
        }
        return "";
    }

    nlohmann::json* GetUpperContractNodePtr(nlohmann::json& node, std::vector<nlohmann::json>& contract_nodes) {
        auto [start_pos, len] = GetStartPosAndLen(node["src"].get<std::string>());
        for (auto& contract_node : contract_nodes) {
            auto [func_start_pos, func_len] = GetStartPosAndLen(contract_node["src"].get<std::string>());
            if (start_pos >= func_start_pos && start_pos + len <= func_start_pos + func_len) {
                return &contract_node;
            }
        }
        return nullptr;
    }

    nlohmann::json* GetUpperFuncNodePtr(nlohmann::json& node, std::vector<nlohmann::json>& func_nodes) {
        auto [start_pos, len] = GetStartPosAndLen(node["src"].get<std::string>());
        for (auto& func_node : func_nodes) {
            auto [func_start_pos, func_len] = GetStartPosAndLen(func_node["src"].get<std::string>());
            if (start_pos >= func_start_pos && start_pos + len <= func_start_pos + func_len) {
                return &func_node;
            }
        }
        return nullptr;
    }

    nlohmann::json* GetFuncBlockNodePtr(nlohmann::json& node) {
        if (node["children"].is_array()) {
            for (size_t i = 0; i < node["children"].size(); i++) {
                auto& child = node["children"][i];
                if (child["name"] == "Block") {
                    return &child;
                }
            }
        }
        return nullptr;
    }

    std::unordered_map<std::string, DefType> GetDefinitionMap(nlohmann::json& root, bool is_recursive = true) {
        std::unordered_map<std::string, DefType> define_map;
        std::queue<nlohmann::json> q;
        q.push(root);
        while (!q.empty()) {
            nlohmann::json node = q.front();
            q.pop();
            if (node.contains("name") && node.contains("attributes") && node.at("attributes").contains("name") && 
                node["attributes"]["name"] != nullptr && node["attributes"]["name"] != "") {
                std::string define_name = node["attributes"]["name"];
                std::string define_type = node["name"];
                if (define_type == "ContractDefinition") {
                    define_map[define_name] = DefType::CONTRACT_DEF;
                } else if (define_type == "FunctionDefinition") {
                    define_map[define_name] = DefType::FUNC_DEF;
                    // if is_recursive == false 不进入函数内部
                    if (!is_recursive) {
                        continue;
                    }
                } else if (define_type == "StructDefinition") {
                    define_map[define_name] = DefType::STRUCT_DEF;
                } else if (define_type == "VariableDeclaration") {
                    define_map[define_name] = DefType::VAR_DEF;
                }
            }
            for (auto& iter : node.items()) {
                if (iter.key() == "children" && iter.value().is_array()) {
                    for (auto& child : iter.value()) {
                        q.push(child);
                    }
                }
            }
        }
        return define_map;
    }

    std::string GetTimeForFileName() {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
        // 转换为time_t结构体
        auto now_c = std::chrono::system_clock::to_time_t(now);
        // 获取微秒
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

        // 将时间转换为字符串格式
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d-%H-%M-%S-") << std::setfill('0') << std::setw(6) << us.count();
        return ss.str();
    }

    std::string file_;
    std::string source_code_;
    std::shared_ptr<AstWrapper> ast_;
    nlohmann::json ast_json_;
    std::string dst_path_;

    std::vector<FuncInfo> secure_info_;
    std::vector<FuncInfo> inject_info_;
    int gen_num_; // 生成的数量
    int secure_num_; // 提取的无漏洞数量
    int max_gen_num_;
    int max_secure_num_;

    int vulnerable_counter_;
    int secure_counter_;

    std::set<std::string> token_flow_set_;

    int repeat_snippet_ = 0;
};

} // namespace svc