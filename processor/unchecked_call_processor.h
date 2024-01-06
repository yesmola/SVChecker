#pragma once

#include "processor/processor.h"

#include "lexer/parser.h"

namespace svc {

class UncheckedCallProcessor : public Processor {
public:
    UncheckedCallProcessor() : Processor() {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    UncheckedCallProcessor(int n) : Processor(n) {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    ~UncheckedCallProcessor() = default;

    std::string GetName() override {
        return "UncheckedCall";
    }

protected:
    int Extract() override {
        spdlog::debug("[UncheckedCallProcessor] Start extracting...");
        return SUCCESS;
    }

    int ExtractInject() override {
        spdlog::debug("[{}] Start extracting inject...", GetName());
        if (ast_ == nullptr) {
            spdlog::error("[{}] ast is nullptr!!", GetName());
            return IOE_INJECT_ERROR;
        }
        if (vulnerable_counter_ >= max_gen_num_) {
            return SUCCESS;
        }
        for (auto& contract_node : ast_->GetNodes()) {
            for (auto& func_node : contract_node->GetChildren()) {
                if (func_node->GetType() != "function") {
                    continue;
                }
                for (auto& child : func_node->GetChildren()) {
                    if (child->GetType() != "function_call") {
                        continue;
                    }
                    auto node = child->GetJsonNode();
                    std::string func_call = child->GetPartCode();
                    if (func_call.find(".call(") == func_call.npos && func_call.find(".call.value(") == func_call.npos &&
                        func_call.find(".delegatecall(") == func_call.npos && func_call.find(".end(") == func_call.npos) {
                        continue;
                    }

                    std::string func_source_code = func_node->GetPartCode();
                    std::string block_soure_code = func_node->GetPartCode();
                    for (auto& line : StringUtils::SplitString(block_soure_code, '\n')) {
                        if (line.find(func_call) != line.npos &&
                        (line.find("require") != line.npos || line.find("if") != line.npos || line.find("=") != line.npos)) {
                            std::shared_ptr<Parser> parser = std::make_shared<Parser>(func_call);
                            parser->ParseProgram();
                            std::set<std::string> ori_set;
                            for (auto& token : parser->GetTokenFlow()) {
                                if (token->GetType() == "IDENT") {
                                    ori_set.insert(token->GetLiteral());
                                }
                            }
                            inject_info_.emplace_back(func_node->GetJsonNode(), line + "@" + func_call, func_node->GetPartCode(), ori_set);
                            secure_info_.emplace_back(func_node->GetJsonNode(), "", func_node->GetPartCode(), ori_set);
                        }
                    }
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
        if (ast_ == nullptr) {
            spdlog::error("[{}] ast is nullptr!!", GetName());
            return IOE_INJECT_ERROR;
        }
        if (secure_counter_ >= max_secure_num_) {
            return SUCCESS;
        }

        spdlog::debug("[{}] End extracting candidate.", GetName());
        return SUCCESS;
    }

    int Inject() {
        spdlog::debug("[{}] Start Injecting...", GetName());
        for (auto& info : inject_info_) {
            std::string line = StringUtils::SplitString(info.line, '@')[0];
            std::string func_call = StringUtils::SplitString(info.line, '@')[1];
            std::string mod_line = line;
            if (mod_line.find("if") != mod_line.npos) {
                mod_line.replace(mod_line.find(func_call), func_call.length(), "true");
                mod_line = func_call + ";\n" + mod_line;
            } else if (mod_line.find("require") != mod_line.npos) {
                mod_line.replace(mod_line.find(func_call), func_call.length(), "true");
                mod_line = func_call + ";\n" + mod_line;
            } else if (mod_line.find("=") != mod_line.npos) {
                std::string left = StringUtils::SplitString(mod_line, '=')[0];
                std::string right = StringUtils::SplitString(mod_line, '=')[1];
                if (left.find(",") != left.npos) {
                    mod_line = func_call + ";\n" + left + "=(true, \"\");\n";
                } else {
                    mod_line = func_call + ";\n" + left + "= true;\n";
                }
            }
            info.code.replace(info.code.find(line), line.length(), mod_line);
        }
        return SUCCESS;
    }

private:
    bool FindLowLevelCall(nlohmann::json& root, const std::string& name, const std::string& type) {
        std::queue<nlohmann::json> q;
        q.push(root);
        while (!q.empty()) {
            nlohmann::json cur = q.front();
            q.pop();
            if (cur["attributes"]["member_name"] == name &&
                cur["attributes"]["referencedDeclaration"] == nullptr &&
                cur["attributes"]["type"] == type) {
                return true;
            }
            for (auto& iter : cur.items()) {
                if (iter.key() == "children" && iter.value().is_array()) {
                    for (auto& child : iter.value()) {
                        q.push(child);
                    }
                }
            }
        }
        return false;
    }
};

} // namespace svc