#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <filesystem>

#include <json/json.hpp>
#include <spdlog/spdlog.h>

#include "processor/common.h"
#include "processor/compiler.h"
#include "processor/int_overflow_processor.h"
#include "processor/access_control_processor.h"
#include "processor/unchecked_call_processor.h"
#include "processor/dos_processor.h"
#include "processor/reentrancy_processor.h"
#include "utils/file_utils.h"
#include "ast/ast_wrapper.h"

namespace svc {
    
class ProcessorManager {
public:
    static ProcessorManager* GetInstance() {
        static ProcessorManager instance;
        return &instance;
    }

    void Init() {
        RegisterInjectors();
    }

    void SetDataPath(const std::string& path, const std::string& dst_path, const std::string& version) {
        data_path_ = path;
        dst_path_ = dst_path;
        version_ = version;
    }

    void Clear() {
        source_files_.clear();
    }

    void PreCompile() {
        std::time_t start_time = std::time(nullptr);
        if (data_path_.empty()) {
            spdlog::error("[ProcessorManager] You must set data path first.");
            return;
        }
        FileUtils::ListFiles(data_path_, source_files_, ".sol");
        spdlog::info("[ProcessorManager] Find {} source files.", source_files_.size());
        // create a tmp directory to cache injected files
        FileUtils::CreateDir(kTmpPath);
        // switch compiler version
        if (Compiler::SwitchVersion(version_)) {
            spdlog::error("[ProcessorManager] Switch version {} success.", version_);
            return;
        } else {
            spdlog::info("[ProcessorManager] Switch version {} success.", version_);
        }

        int counter = 0;
        std::ofstream f("./rm_file.sh");
        for (auto& file : source_files_) {
            spdlog::info("[ProcessorManager] Process {}/{} file: {}", ++counter, source_files_.size(), file);

            std::string file_name = StringUtils::SplitString(file, '/').back();
            if (std::filesystem::exists("/mnt/d/ASTset/" + file_name + "_json.ast")) {
                continue;
            }
            std::string source_code;
            FileUtils::ReadFileToString(file, source_code);
            spdlog::debug("[ProcessorManager] Read file successfully.");
            // compile source code
            nlohmann::json ast_json;
            int ret = Compiler::CompileToJson(source_code, ast_json);
            if (ret) {
                spdlog::error("[ProcessorManager] Compile {} failed. ERROR CODE: {}", file, ret);
                f << "rm " << file << std::endl;
                continue;
            }

            ShellUtils::RunCommand("cp " + kTmpJsonFile + " /mnt/d/ASTset/" + file_name + "_json.ast");
        }
        f.flush();
        f.close();
        spdlog::info("[ProcessorManager] Excute time = {}s", std::to_string(std::time(nullptr) - start_time));
        return;
    }

    void InjectBug(bool pre_compiled) {
        std::time_t start_time = std::time(nullptr);
        if (data_path_.empty()) {
            spdlog::error("[ProcessorManager] You must set data path first.");
            return;
        }
        // get all files in dataset path
        FileUtils::ListFiles(data_path_, source_files_, ".sol");
        spdlog::info("[ProcessorManager] Find {} source files.", source_files_.size());
        // create a tmp directory to cache injected files
        FileUtils::CreateDir(kTmpPath);
        // switch compiler version
        if (Compiler::SwitchVersion(version_)) {
            spdlog::error("[ProcessorManager] Switch version {} success.", version_);
            return;
        } else {
            spdlog::info("[ProcessorManager] Switch version {} success.", version_);
        }

        // for every file and every injector, inject bug
        std::ofstream f("./rm_file.sh");
        int counter = 0;

        std::string check_point = "";
        bool run = false;
        for (auto& file : source_files_) {
            counter++;
            if (!run && check_point != "" && file != check_point) {
                continue;
            } else {
                run = true;
            }
            spdlog::info("[ProcessorManager] Process {}/{} file: {}", counter, source_files_.size(), file);
            std::string source_code;
            FileUtils::ReadFileToString(file, source_code);
            spdlog::debug("[ProcessorManager] Read file successfully.");
            // compile source code
            nlohmann::json ast_json;
            std::string file_name = StringUtils::SplitString(file, '/').back();
            if (!pre_compiled) {
                int ret = Compiler::CompileToJson(source_code, ast_json);
                if (ret) {
                    spdlog::error("[ProcessorManager] Compile {} failed. ERROR CODE: {}", file, ret);
                    f << "rm " << file << std::endl;
                    continue;
                }
                spdlog::debug("[ProcessorManager] new compiled.");
            } else {
                Compiler::CleanSourceCode(source_code);
                if (!std::filesystem::exists("/mnt/d/ASTset/" + file_name + "_json.ast")) {
                    spdlog::error("[ProcessorManager] Precompiled But File Not Exist. file = {}",
                                    "/mnt/d/ASTset/" + file_name + "_json.ast");
                    f << "rm " << file << std::endl;
                    continue;
                }
                std::ifstream ast_json_file("/mnt/d/ASTset/" + file_name + "_json.ast");
                ast_json = nlohmann::json::parse(ast_json_file);
            }
            auto ast = std::make_shared<AstWrapper>(ast_json, source_code);
            ast->Parse();
            if (ast->GetNodes().size() == 0) {
                continue;
            }
            spdlog::debug("[ProcessorManager] Compile successfully.");
            // inject bug

            size_t finished = 0;
            for (auto& processor : processors_) {
                spdlog::debug("[ProcessorManager] {}", processor->GetName());
                if (processor->Finish()) {
                    finished ++;
                    continue;
                }
                processor->Init(file_name, ast, dst_path_);
                // if (processor->BuildVulnerbaleDataset() == SUCCESS) {
                // }
                if (processor->BuildDataset() == SUCCESS) {
                }
            }
            if (finished >= processors_.size()) {
                break;
            }
        }

        f.flush();
        f.close();

        PrintStatus();

        spdlog::info("[ProcessorManager] Excute time = {}s", std::to_string(std::time(nullptr) - start_time));
    }

    ProcessorManager(const ProcessorManager&) = delete;
    ProcessorManager(ProcessorManager&&) = delete;
    ProcessorManager operator=(const ProcessorManager&) = delete;
    ProcessorManager operator=(ProcessorManager&&) = delete;
    ~ProcessorManager() = default;

private:
    ProcessorManager() = default;

    void RegisterInjectors() {
        processors_.push_back(new IntOverflowProcessor());
        processors_.push_back(new AccessControlProcessor());
        processors_.push_back(new UncheckedCallProcessor());
        processors_.push_back(new DosProcessor());
        processors_.push_back(new ReentrancyProcessor());
    }

    void PrintStatus() {
        // 表头
        std::cout << std::left << std::setw(20) << "Processor" << std::setw(10) << "Inject" << std::setw(10) << "Secure" << std::setw(10) << "Repeat" << std::endl;
        // 分割线
        std::cout << std::setfill('-') << std::setw(60) << "-" << std::endl;
        for (auto &processor : processors_) {
            std::cout << std::setfill(' ') << std::left << std::setw(20) << processor->GetName()
                << std::setw(10) << processor->GetGenNum() << std::setw(10) << processor->GetSecureNum() << std::setw(10) << processor->GetRepeatNum() << std::endl;
        }
    }

private:
    std::string data_path_;
    std::string version_;
    std::string dst_path_;
    std::vector<std::string> source_files_;
    std::vector<Processor*> processors_;
};

} // namespace svc
