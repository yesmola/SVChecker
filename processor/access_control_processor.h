#pragma once

#include "processor/processor.h"

namespace svc {

class AccessControlProcessor : public Processor {
public:
    AccessControlProcessor() : Processor() {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    AccessControlProcessor(int n) : Processor(n) {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    ~AccessControlProcessor() = default;

    std::string GetName() override {
        return "AccessControl";
    }

protected:
    int Extract() override {
        return SUCCESS;
    }

    int ExtractInject() override {
        spdlog::debug("[{}] Start extracting inject...", GetName());
        if (ast_ == nullptr) {
            spdlog::error("[{}] ast is nullptr!!", GetName());
            return ACE_INJECT_ERROR;
        }
        if (vulnerable_counter_ >= max_gen_num_ && secure_counter_ >= max_secure_num_) {
            return SUCCESS;
        }
        for (auto& contract_node : ast_->GetNodes()) {
            for (auto& func_node : contract_node->GetChildren()) {
                if (func_node->GetType() != "function") {
                    continue;
                }
                nlohmann::json node = func_node->GetJsonNode();
                std::string func_source_node = func_node->GetPartCode();
                std::string first_line = StringUtils::SplitString(func_source_node, '\n')[0];

                std::string inject_line = "";
                if (first_line.find(" onlyOwner ") != first_line.npos) {
                    inject_line = " onlyOwner ";
                } else if (first_line.find(" onlyowner ") != first_line.npos) {
                    inject_line = " onlyowner ";
                } else if (first_line.find(" onlyowner()") != first_line.npos) {
                    inject_line = " onlyowner()";
                } else if (first_line.find(" onlyOwner()") != first_line.npos) {
                    inject_line = " onlyOwner()";
                }
                if (inject_line == "") {
                    std::string block_souce_code =func_node->GetBlockCode();
                    if (block_souce_code.find("\n") != block_souce_code.npos) {
                        std::vector<std::string> block_lines = StringUtils::SplitString(func_node->GetBlockCode(), '\n');
                        std::regex re1("require\\s*\\(.+==\\s*msg\\.sender");
                        std::regex re2("require\\s*\\(\\s*msg\\.sender\\s*==");
                        for (size_t i = 0; i < block_lines.size(); ++i) {
                            if (i >= 3) {
                                break;
                            }
                            if (std::regex_search(block_lines[i], re1) || std::regex_search(block_lines[i], re2)) {
                                inject_line = block_lines[i];
                                break;
                            }
                        }
                    }
                }
                if (inject_line == "") {
                    continue;
                }
                for (auto& child : func_node->GetChildren()) {
                    if (child->GetType() != "assignment") {
                        continue;
                    }
                    nlohmann::json assign_node = child->GetJsonNode();
                    if (assign_node["attributes"]["operator"] != "=" || assign_node["attributes"]["type"] != "address") {
                        continue;
                    }
                    nlohmann::json left_node = assign_node["children"][0];
                    nlohmann::json right_node = assign_node["children"][1];
                    std::set<int> ids = func_node->GetIds();
                    int ref_id = 0;
                    if (left_node["attributes"].contains("referencedDeclaration") &&
                        left_node["attributes"]["referencedDeclaration"] != nullptr) {
                        ref_id = left_node["attributes"]["referencedDeclaration"];
                    }
                    if (ref_id == 0 || ids.find(ref_id) != ids.end()) {
                        continue;
                    }

                    std::string left = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    std::string right = source_code_.substr(GetStartPosAndLen(right_node["src"]).first, GetStartPosAndLen(right_node["src"]).second);
                    std::set<std::string> ori_set = {left, right};
                    inject_info_.emplace_back(func_node->GetJsonNode(), inject_line, func_source_node, ori_set);
                    secure_info_.emplace_back(func_node->GetJsonNode(), inject_line + "@" + left, func_source_node, ori_set);
                }
            }
        }
        gen_num_ += inject_info_.size();
        secure_num_ += secure_info_.size();
        spdlog::debug("[{}] End extracting inject.", GetName());
        return SUCCESS;
    }

    int ExtractCanditate() override {
        spdlog::debug("[{}] Start extracting candidate...", GetName());
        spdlog::debug("[{}] End extracting candidate.", GetName());
        return SUCCESS;
    }

    int Inject() {
        spdlog::debug("[{}] Start Injecting...", GetName());
        for (auto& info : inject_info_) {
            if (info.line.find("require") != info.line.npos) {
                info.code.replace(info.code.find(info.line), info.line.length(), "");
                continue;
            }
            std::vector<std::string> code_part = StringUtils::SplitString(info.code, '\n');
            std::string code = code_part[0] + "\n";
            code.replace(code.find(info.line), info.line.length(), "");
            for (size_t i = 1; i < code_part.size(); i++) {
                code += code_part[i] + "\n";
            }
            info.code = code;
        }
        for (auto& info : secure_info_) {
            std::string line = StringUtils::SplitString(info.line, '|')[0];
            std::string left = StringUtils::SplitString(info.line, '|')[1];
            if (line.find("onlyOwner") != line.npos) {
                info.code = "modifier onlyOnwer { reuqire(" + left + " == msg.sender); }\n" + info.code;
            }
            if (line.find("onlyowner") != line.npos) {
                info.code = "modifier onlyowner { reuqire(" + left + " == msg.sender); }\n" + info.code;
            }
        }
        return SUCCESS;
    }
};

} // namespace svc