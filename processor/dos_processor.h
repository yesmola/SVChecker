#pragma once

#include <cctype>

#include "processor/processor.h"

namespace svc {

class DosProcessor : public Processor {
public:
    DosProcessor() : Processor() {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    DosProcessor(int n) : Processor(n) {
        vulnerable_counter_ = 1;
        secure_counter_ = 1;
    }
    ~DosProcessor() = default;

    std::string GetName() override {
        return "DoS";
    }

protected:
    int Extract() override {
        spdlog::debug("[DosProcessor] Start extracting...");
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
            for (auto& func_node : contract_node->GetChildren()) {
                if (func_node->GetType() != "function") {
                    continue;
                }
                std::string func_source_code = func_node->GetPartCode();
                std::vector<std::string> line_code = StringUtils::SplitString(func_source_code, '\n');
                // for () {/*heavy code*/}
                std::regex re("for\\s*\\(\\s*uint\\d*");
                for (size_t line_no = 0; line_no < line_code.size(); line_no ++) {
                    if (line_no > 0 && std::regex_search(line_code[line_no], re)) {
                        std::vector<std::string> line_part = StringUtils::SplitString(line_code[line_no], ';');
                        if (line_part.size() != 3) {
                            continue;
                        }
                        std::string line = line_part[1];
                        // 找到for循环的控制变量
                        size_t start_pos = 0;
                        for (start_pos = 0; start_pos < line.size(); start_pos ++) {
                            if (line[start_pos] == '>' || line[start_pos] == '<') {
                                break;
                            }
                        }
                        while (start_pos < line.size() && !isalpha(line[start_pos])) {
                            start_pos ++;
                        }
                        int len = 0;
                        while (start_pos + len < line.size() && line[start_pos + len] != ' ' && line[start_pos + len] != ';' &&
                               line[start_pos + len] != '&' && line[start_pos + len] != '|') {
                            len ++;
                        }
                        if (len == 0) {
                            continue;
                        }
                        std::string var = line.substr(start_pos, len);
                        // 判断之前是否是对该变量的检查
                        for (size_t prev = 0; prev < line_no; prev ++) {
                            std::string prev_line = line_code[prev];
                            if ((prev_line.find("if") != prev_line.npos || prev_line.find("require") != prev_line.npos) && 
                                prev_line.find(var) != prev_line.npos) {
                                std::set<std::string> ori_set = {var};
                                inject_info_.emplace_back(func_node->GetJsonNode(), prev_line, func_source_code, ori_set);
                                secure_info_.emplace_back(func_node->GetJsonNode(), "", func_source_code, ori_set);
                                break;
                            }
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

    int Inject() {
        spdlog::debug("[{}] Start Injecting...", GetName());
        for (auto& info : inject_info_) {
            if (info.line.find("if") != info.line.npos) {
                info.code.replace(info.code.find(info.line), info.line.length(), "if (true) {");
            } else {
                info.code.replace(info.code.find(info.line), info.line.length(), "");
            }
        }
        return SUCCESS;
    }
};

} // namespace svc