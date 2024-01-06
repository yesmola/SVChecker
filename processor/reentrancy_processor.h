#pragma once

#include "processor/processor.h"

namespace svc {

class ReentrancyProcessor : public Processor {
public:
    ReentrancyProcessor() : Processor() {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    ReentrancyProcessor(int n) : Processor(n) {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    ~ReentrancyProcessor() = default;

    std::string GetName() override {
        return "Reentrancy";
    }

protected:
    int Extract() override {
        spdlog::debug("[ReentrancyProcessor] Start extracting...");
        std::vector<nlohmann::json> function_def_nodes = GetFunctionDefinitionNodes(ast_json_);
        std::vector<nlohmann::json> assignment_nodes = GetAssignmentNodes(ast_json_);

        for (auto& node : assignment_nodes) {
            if (vulnerable_counter_ >= max_gen_num_) {
                break;
            }
            if ((node["attributes"]["operator"] == "-=" || node["attributes"]["operator"] == "=") && IsUint(node["attributes"]["type"]) && 
                node["children"].is_array() && node["children"].size() == 2) {
                auto &left_node = node["children"][0];
                auto &right_node = node["children"][1];
                std::string assign_line = source_code_.substr(GetStartPosAndLen(node["src"]).first, GetStartPosAndLen(node["src"]).second);
                if ((node["attributes"]["operator"] == "-=" && IsUint(left_node["attributes"]["type"]) && IsUint(right_node["attributes"]["type"])) ||
                    (node["attributes"]["operator"] == "=" && IsUint(left_node["attributes"]["type"]) && right_node["attributes"]["value"] == "0")) {
                    std::string left = source_code_.substr(GetStartPosAndLen(left_node["src"]).first, GetStartPosAndLen(left_node["src"]).second);
                    std::string right = source_code_.substr(GetStartPosAndLen(right_node["src"]).first, GetStartPosAndLen(right_node["src"]).second);
                    nlohmann::json* func_node_ptr = GetUpperFuncNodePtr(node, function_def_nodes);
                    if (func_node_ptr == nullptr) {
                        continue;
                    }
                    std::string func_source_code = source_code_.substr(GetStartPosAndLen((*func_node_ptr)["src"]).first,
                                                                       GetStartPosAndLen((*func_node_ptr)["src"]).second);
                    
                    nlohmann::json* block_node_ptr = GetFuncBlockNodePtr(*func_node_ptr);
                    if (block_node_ptr == nullptr) {
                        continue;
                    }
                    std::string block_source_code = source_code_.substr(GetStartPosAndLen((*block_node_ptr)["src"]).first,
                                                                        GetStartPosAndLen((*block_node_ptr)["src"]).second);

                    std::vector<std::string> line_code = StringUtils::SplitString(block_source_code, '\n');
                    // 如果left是一个数组 找到地址
                    if (left.find("[") != left.npos) {
                        std::vector<std::string> left_part = StringUtils::SplitString(left, '[');
                        left = left_part.back().substr(0, left_part.back().size() - 1);
                    }
                    for (auto line : line_code) {
                        if ((line.find(left) != line.npos || (right != "0" && line.find(right) != line.npos)) && 
                            (line.find("Transfer(") != line.npos || line.find(".transfer(") != line.npos || line.find(".send(") != line.npos)) {
                            std::set<std::string> ori_set = {left, right};
                            inject_info_.emplace_back(*func_node_ptr, assign_line + "@" + line, func_source_code, ori_set);
                            secure_info_.emplace_back(*func_node_ptr, "", func_source_code, ori_set);
                            break;
                        }
                    }
                }
            }
        }
        // for (auto& node : function_def_nodes) {
        //     if (secure_counter_ >= max_secure_num_) {
        //         break;
        //     }
        //     std::string func_source_code = source_code_.substr(GetStartPosAndLen(node["src"]).first,
        //                                                        GetStartPosAndLen(node["src"]).second);
        //     if (func_source_code.find(".call.value(") != func_source_code.npos ||
        //         func_source_code.find(".call(") != func_source_code.npos) {
        //         secure_info_.emplace_back(node, "", func_source_code);
        //     }
        // }
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
        Extract();
        gen_num_ += inject_info_.size();
        secure_num_ += secure_info_.size();
        spdlog::debug("[{}] End extracting inject.", GetName());
        return SUCCESS;
    }

    int Inject() {
        spdlog::debug("[{}] Start Injecting...", GetName());
        for (auto& info : inject_info_) {
            std::vector<std::string> line_part = StringUtils::SplitString(info.line, '@');
            std::string assgin_line = line_part[0];
            std::string line = line_part[1];
            // 解析assgin_line
            size_t l = 0;
            for (; l < assgin_line.size(); l ++) {
                if (assgin_line[l] == ' ' || assgin_line[l] == '-' || assgin_line[l] == '=') {
                    break;
                }
            }
            size_t r = assgin_line.size() - 1;
            for (; r >= 0; r --) {
                if (assgin_line[r] == ' ' || assgin_line[r] == '=') {
                    break;
                }
            }
            std::string left = assgin_line.substr(0, l);
            std::string right = assgin_line.substr(r + 1);
            // 如果left是一个数组
            if (left.find("[") != left.npos) {
                std::vector<std::string> left_part = StringUtils::SplitString(left, '[');
                left = left_part.back().substr(0, left_part.back().size() - 1);
            }
            info.code.replace(info.code.find(line), line.size(), "");
            if (right == "0") {
                info.code.replace(info.code.find(assgin_line), assgin_line.size(), "uint VAR0;\n msg.sender.call.value(VAR0);\n" + assgin_line);
            } else {
                info.code.replace(info.code.find(assgin_line), assgin_line.size(), left + ".call.value(" + right + ");\n" + assgin_line);
            }
        }
        return SUCCESS;
    }
};

} // namespace svc