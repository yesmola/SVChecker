#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <random>

#include "base.h"
#include "json.hpp"

#include "labeler.h"

namespace solver {

void Labeler::ReadFile() {
    // "Integer Underflow."
    // "Integer Overflow."
    // "Timestamp Dependency."
    // "Re-Entrancy Vulnerability."
    // "Callstack Depth Attack Vulnerability."
    // "Parity Multisig Bug 2."
    using nlohmann::json;
    std::ifstream i(result_dir + file_name_ + "/result.json");
    json j;
    i >> j;
    json analysis = j["analysis"];

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 10.0);

    for (json::iterator it = analysis.begin(); it != analysis.end(); ++ it) {
        json contract_ana = *it;
        // std::cout << contract_error["name"] << std::endl;
        // std::cout << contract_error["errors"] << std::endl;
        json errors = contract_ana["errors"];
        for (json::iterator e_it = errors.begin(); e_it != errors.end(); ++ e_it) {
            json error = *e_it;
            std::string message = error["message"];
            /*if (message == "Integer Underflow.") {
                if (dist(mt) > 4.0) continue;
            }
            if (message == "Integer Overflow.") {
                if (dist(mt) > 4.0) continue;
            }*/
            /* if (message == "Timestamp Dependency.") n_td_++;
            if (message == "Re-Entrancy Vulnerability.") n_re_++;
            if (message == "Callstack Depth Attack Vulnerability.") n_cd_++;*/
            int e_line = error["line"];
            error_lines_.insert(e_line);
        }
    }
}

bool Labeler::AddLabel(std::string& source, std::vector<std::vector<Token*>>& code_patches, std::ofstream& ofile) {
    /*int n = code_patches.size();
    for (int i = 0; i < n; ++ i) {
        for (int j = i; j < n; ++ j) {
            if (code_patches[i][0].GetLine() > code_patches[j][0].GetLine()) {
                std::swap(code_pathes[i], code_pathes[j]);
            }
        }
    }*/
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 10.0);
    bool flag = false;
    for (auto p : code_patches) {
        bool vulnerable = true;
        std::map<int64_t, std::vector<std::string>> program;
        for (auto t : p) {
            program[t->GetLine()].push_back(t->GetLiteral());
            if (error_lines_.find(t->GetLine()) != error_lines_.end()) {
                vulnerable = true;
            }
        }
        //if (vulnerable || (!vulnerable && dist(mt) > 9.3 && program.size() < 30)) {
        if (true) {
            ofile << source << std::endl;
            for (auto it = program.begin(); it!=program.end(); ++it) {
                int64_t line = it->first;
                auto tokens = it->second;
                ofile << line << " ";
                for (auto t : tokens) {
                    ofile << t << " ";
                }
                ofile << std::endl;
            }
            if (vulnerable) {
                flag = true;
                n_malicious_++;
                ofile << 1 << std::endl;
            } else {
                n_benign_++;
                ofile << 0 << std::endl;
            }
            ofile << "----------------------------------------" << std::endl;
        }
    }
    return flag;
}


} // namespace solver