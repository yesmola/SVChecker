/*
Build dataset from smart-contract-sanctuary-ethereum
https://github.com/tintinweb/smart-contract-sanctuary-ethereum/tree/master/contracts/mainnet
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <json/json.hpp>

#include "processor/common.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "utils/shell_utils.h"

const std::string kSanctuaryPath = "/mnt/d/smart-contract-sanctuary-ethereum/contracts/rinkeby/";
const std::string kDestPath = "/home/yuanye/graduation/Dataset/";

int main() {
    std::string contracts;
    svc::FileUtils::ReadFileToString(kSanctuaryPath + "contracts.json", contracts);
    std::vector<std::string> contract_vec = svc::StringUtils::SplitString(contracts, '\n'); 

    int counter = 0;
    for (auto &contract_json : contract_vec) {
        counter++;
        if (counter % 10000 == 1) {
            std::cout << counter << "/" << contract_vec.size() << std::endl;
        }
        nlohmann::json contract;
        try {
            contract = nlohmann::json::parse(contract_json);
        } catch(...) {
            std::cout << "error " << std::endl;
            continue;
        }
        std::string address = contract["address"];
        std::string version = contract["compiler"];
        std::string name = contract["name"];
        // compiler: 0.5.x | v0.5.x
        if (version[0] != '0') {
            version = version.substr(1);
        }
        
        std::string source_dictory = svc::StringUtils::StrToLower(address.substr(2, 2));
        std::string source_file = kSanctuaryPath + source_dictory + "/" + address.substr(2) + "_" + name + ".sol";

        if (!svc::FileUtils::IsFileExists(source_file)) {
            continue;
        }

        if (version[2] == '4') {
            // svc::ShellUtils::RunCommand("cp " + source_file + " " + kDestPath + "4/");
        } else if (version[2] == '5') {
            // svc::ShellUtils::RunCommand("cp " + source_file + " " + kDestPath + "5/");
        } else if (version[2] == '6') {
            svc::ShellUtils::RunCommand("cp " + source_file + " " + kDestPath + "6/");
        }
    }
    return 0;
}