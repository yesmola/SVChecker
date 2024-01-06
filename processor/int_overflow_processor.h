#pragma once

#include "processor/processor.h"

namespace svc {

class IntOverflowProcessor : public Processor {
public:
    IntOverflowProcessor() : Processor() {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    IntOverflowProcessor(int n) : Processor(n) {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    ~IntOverflowProcessor() = default;

    std::string GetName() override {
        return "IntOverflow";
    }

protected:
    int Extract() override {
        return SUCCESS;
    }

    int ExtractInject() override {
        spdlog::debug("[{}] Start extracting inject...", GetName());
        if (ast_ == nullptr) {
            spdlog::error("[{}] ast is nullptr!!", GetName());
            return IOE_INJECT_ERROR;
        }
        if (vulnerable_counter_ >= max_gen_num_ && secure_counter_ >= max_secure_num_) {
            return SUCCESS;
        }
        for (auto& contract_node : ast_->GetNodes()) {
            if (contract_node->GetName() == "SafeMath" || contract_node->GetName() == "safeMath") {
                continue;
            }
            for (auto& func_node : contract_node->GetChildren()) {
                if (func_node->GetType() != "function") {
                    continue;
                }
                InjectOneFunction(func_node);
            }
        }
        gen_num_ += inject_info_.size();
        secure_num_ += secure_info_.size();
        spdlog::debug("[{}] End extracting inject.", GetName());
        return SUCCESS;
    }

    int ExtractCanditate() override {
        spdlog::debug("[{}] Start extracting candidate...", GetName());
        if (ast_ == nullptr) {
            spdlog::error("[{}] ast is nullptr!!", GetName());
            return IOE_INJECT_ERROR;
        }
        if (secure_counter_ >= max_secure_num_) {
            return SUCCESS;
        }
        for (auto& contract_node : ast_->GetNodes()) {
            for (auto& func_node : contract_node->GetChildren()) {
                if (func_node->GetType() != "function") {
                    continue;
                }
                CanditateOneFunction(func_node);
            }
        }
        spdlog::debug("[{}] End extracting candidate.", GetName());
        return SUCCESS;
    }

    int Inject() override {
        spdlog::debug("[{}] Start Injecting...", GetName());
        for (auto& info : inject_info_) {
            while (info.code.find(".add(") != info.code.npos) {
                info.code.replace(info.code.find(".add("), 5, "+(");
            }
            while (info.code.find(".sub(") != info.code.npos) {
                info.code.replace(info.code.find(".sub("), 5, "-(");
            }
            while (info.line.find(".add(") != info.code.npos) {
                info.line.replace(info.line.find(".add("), 5, "+(");
            }
            while (info.line.find(".sub(") != info.code.npos) {
                info.line.replace(info.line.find(".sub("), 5, "-(");
            }
            if (info.line.find("if") != info.line.npos) {
                info.code.replace(info.code.find(info.line), info.line.length(), "if (true) {");
            } else {
                info.code.replace(info.code.find(info.line), info.line.length(), "");
            }
        }
        return SUCCESS;
    }

private:
    void InjectOneFunction(std::shared_ptr<Node> func_node) {
        std::string block_source_code = func_node->GetBlockCode();
        for (auto& child : func_node->GetChildren()) {
            if (child->GetType() == "assignment") {
                // uint256 c; c = a + b; c = a - b;
                nlohmann::json node = child->GetJsonNode();
                if (node["attributes"]["operator"] == "=" && IsUint(node["attributes"]["type"]) &&
                    node["children"].is_array() && node["children"].size() == 2) {
                    auto &left_node = node["children"][0]; // c
                    std::string c = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    auto &right_node = node["children"][1]; // a + b
                    if (right_node["children"].size() == 2 && IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"]) &&
                        (right_node["attributes"]["operator"] == "+" || right_node["attributes"]["operator"] == "-")) {
                        auto &sub_left_node = right_node["children"][0]; // a
                        auto &sub_right_node = right_node["children"][1]; // b
                        if (IsUint(sub_left_node["attributes"]["type"]) && IsUint(sub_right_node["attributes"]["type"])) {
                            std::string a = source_code_.substr(GetStartPosAndLen(sub_left_node["src"]).first, GetStartPosAndLen(sub_left_node["src"]).second);
                            std::string b = source_code_.substr(GetStartPosAndLen(sub_right_node["src"]).first, GetStartPosAndLen(sub_right_node["src"]).second);
                            std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                            for (auto line : line_code) {
                                if (line.find(c) != line.npos && (line.find(a) != line.npos || line.find(b) != line.npos) && 
                                    (line.find(">") != line.npos || line.find("<") != line.npos)) {
                                    std::set<std::string> ori_set = {a, b, c};
                                    inject_info_.emplace_back(func_node->GetJsonNode(), line, func_node->GetPartCode(), ori_set);
                                    secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                                    break;
                                }
                            }
                        }
                    }
                } else if ((node["attributes"]["operator"] == "+=" || node["attributes"]["operator"] == "-=") && 
                            IsUint(node["attributes"]["type"]) && node["children"].is_array() && node["children"].size() == 2) {
                    // c += a; c -= a;
                    auto &left_node = node["children"][0]; // c
                    auto &right_node = node["children"][1]; // a
                    std::string c = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    std::string a = source_code_.substr(GetStartPosAndLen(right_node["src"]).first, GetStartPosAndLen(right_node["src"]).second);
                    if (IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"])) {
                        // 找到对c a的约束
                        std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                        for (auto line : line_code) {
                            if (line.find(c) != line.npos && line.find(a) != line.npos && (line.find(">") != line.npos || line.find("<") != line.npos)) {
                                std::set<std::string> ori_set = {a, c};
                                inject_info_.emplace_back(func_node->GetJsonNode(), line, func_node->GetPartCode(), ori_set);
                                secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                                break;
                            }
                        }
                    }
                }
            } else if (child->GetType() == "var_statement") {
                nlohmann::json node = child->GetJsonNode();
                // uint256 c = a + b; uint256 c = a - b;
                if (node["children"].is_array() && node["children"].size() == 2) {
                    auto &left_node = node["children"][0]; // uint256 c
                    std::string c = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    c = StringUtils::SplitString(c, ' ').back();
                    auto &right_node = node["children"][1]; // a + b
                    if (IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"]) &&
                        (right_node["attributes"]["operator"] == "+" || right_node["attributes"]["operator"] == "-")) {
                        auto &sub_left_node = right_node["children"][0]; // a
                        auto &sub_right_node = right_node["children"][1]; // b
                        std::string a = source_code_.substr(GetStartPosAndLen(sub_left_node["src"]).first, GetStartPosAndLen(sub_left_node["src"]).second);
                        std::string b = source_code_.substr(GetStartPosAndLen(sub_right_node["src"]).first, GetStartPosAndLen(sub_right_node["src"]).second);
                        if (IsUint(sub_left_node["attributes"]["type"]) && IsUint(sub_right_node["attributes"]["type"])) {
                            // 找到对c a b的约束
                            std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                            for (auto line : line_code) {
                                if (line.find(c) != line.npos && (line.find(a) != line.npos || line.find(b) != line.npos) && 
                                    (line.find(">") != line.npos || line.find("<") != line.npos)) {
                                    std::set<std::string> ori_set = {a, b, c};
                                    inject_info_.emplace_back(func_node->GetJsonNode(), line, func_node->GetPartCode(), ori_set);
                                    secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                                    break;
                                }
                            }
                        }        
                    }
                }
            } else if (child->GetType() == "function_call") {
                std::string part_code = child->GetPartCode();
                auto node = child->GetJsonNode();
                if (node["attributes"].contains("type") && IsUint(node["attributes"]["type"]) && node["children"].size() == 2) {
                    if (node["children"][0]["attributes"].contains("member_name") && 
                        (node["children"][0]["attributes"]["member_name"] == "add" ||
                        node["children"][0]["attributes"]["member_name"] == "sub")) {
                        std::string a;
                        std::string b;
                        std::string tmp = part_code.substr(0, part_code.size() - 1);
                        if (tmp.find(".add (") != tmp.npos) {
                            tmp.replace(tmp.find(".add ("), 6, ".add(");
                        } else if (tmp.find(".sub (") != tmp.npos) {
                            tmp.replace(tmp.find(".sub ("), 6, ".sub(");
                        }
                        if (tmp.find(".add(") != tmp.npos) {
                            tmp.replace(tmp.find(".add("), 5, "~");
                        } else if (tmp.find(".sub(") != tmp.npos) {
                            tmp.replace(tmp.find(".sub("), 5, "~");
                        }
                        if (tmp.size() != 2) {
                            continue;
                        }
                        a = StringUtils::SplitString(tmp, '~')[0];
                        b = StringUtils::SplitString(tmp, '~')[1];
                        std::set<std::string> ori_set = {a, b};
                        inject_info_.emplace_back(func_node->GetJsonNode(), part_code, func_node->GetPartCode(), ori_set);
                        secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                    }
                }
            }
        }
        return;
    }

    void CanditateOneFunction(std::shared_ptr<Node> func_node) {
        for (auto& child : func_node->GetChildren()) {
            std::string block_source_code = func_node->GetBlockCode();
            if (child->GetType() == "assignment") {
                // uint256 c; c = a + b; c = a - b;
                nlohmann::json node = child->GetJsonNode();
                if (node["attributes"]["operator"] == "=" && IsUint(node["attributes"]["type"]) &&
                    node["children"].is_array() && node["children"].size() == 2) {
                    auto &left_node = node["children"][0]; // c
                    std::string c = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    auto &right_node = node["children"][1]; // a + b
                    if (right_node["children"].size() == 2 && IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"]) &&
                        (right_node["attributes"]["operator"] == "+" || right_node["attributes"]["operator"] == "-")) {
                        auto &sub_left_node = right_node["children"][0]; // a
                        auto &sub_right_node = right_node["children"][1]; // b
                        if (IsUint(sub_left_node["attributes"]["type"]) && IsUint(sub_right_node["attributes"]["type"])) {
                            std::string a = source_code_.substr(GetStartPosAndLen(sub_left_node["src"]).first, GetStartPosAndLen(sub_left_node["src"]).second);
                            std::string b = source_code_.substr(GetStartPosAndLen(sub_right_node["src"]).first, GetStartPosAndLen(sub_right_node["src"]).second);
                            std::set<std::string> ori_set = {a, b, c};
                            std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                            for (auto line : line_code) {
                                if (line.find(c) != line.npos && (line.find(a) != line.npos || line.find(b) != line.npos) && 
                                    (line.find(">") != line.npos || line.find("<") != line.npos)) {
                                    std::set<std::string> ori_set = {a, b, c};
                                    secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                                    break;
                                }
                            }
                        }
                    }
                } else if ((node["attributes"]["operator"] == "+=" || node["attributes"]["operator"] == "-=") && 
                            IsUint(node["attributes"]["type"]) && node["children"].is_array() && node["children"].size() == 2) {
                    // c += a; c -= a;
                    auto &left_node = node["children"][0]; // c
                    auto &right_node = node["children"][1]; // a
                    std::string c = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    std::string a = source_code_.substr(GetStartPosAndLen(right_node["src"]).first, GetStartPosAndLen(right_node["src"]).second);
                    if (IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"])) {
                        std::set<std::string> ori_set = {a, c};
                        // 找到对c a的约束
                        std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                        for (auto line : line_code) {
                            if (line.find(c) != line.npos && line.find(a) != line.npos && (line.find(">") != line.npos || line.find("<") != line.npos)) {
                                std::set<std::string> ori_set = {a, c};
                                secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                                break;
                            }
                        }
                        
                    }
                }
            } else if (child->GetType() == "var_statement") {
                nlohmann::json node = child->GetJsonNode();
                // uint256 c = a + b; uint256 c = a - b;
                if (node["children"].is_array() && node["children"].size() == 2) {
                    auto &left_node = node["children"][0]; // uint256 c
                    std::string c = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    c = StringUtils::SplitString(c, ' ').back();
                    auto &right_node = node["children"][1]; // a + b
                    if (IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"]) &&
                        (right_node["attributes"]["operator"] == "+" || right_node["attributes"]["operator"] == "-")) {
                        auto &sub_left_node = right_node["children"][0]; // a
                        auto &sub_right_node = right_node["children"][1]; // b
                        std::string a = source_code_.substr(GetStartPosAndLen(sub_left_node["src"]).first, GetStartPosAndLen(sub_left_node["src"]).second);
                        std::string b = source_code_.substr(GetStartPosAndLen(sub_right_node["src"]).first, GetStartPosAndLen(sub_right_node["src"]).second);
                        if (IsUint(sub_left_node["attributes"]["type"]) && IsUint(sub_right_node["attributes"]["type"])) {
                            std::set<std::string> ori_set = {a, b, c};
                            // 找到对c a b的约束
                            std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                            for (auto line : line_code) {
                                if (line.find(c) != line.npos && (line.find(a) != line.npos || line.find(b) != line.npos) && 
                                    (line.find(">") != line.npos || line.find("<") != line.npos)) {
                                    std::set<std::string> ori_set = {a, b, c};
                                    secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                                    break;
                                }
                            }
                        }        
                    }
                }
            }
        }
        secure_num_ += secure_info_.size();
        return;
    }
};
} // namespace svc