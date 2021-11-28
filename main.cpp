#include <iostream>
#include <fstream>
#include <string>

#include "base.h"
#include "lexer.h"
#include "parser.h"
#include "labeler.h"


int main(int argc, char *argv[]) {
    std::ifstream contract_list("contract_list_ul.txt");
    std::string file_name;
    // std::ofstream dataset("dataset_re.txt", std::ios_base::app);
    // std::ofstream test_list("dataset5.txt", std::ios_base::app);
    // std::ofstream dataset;
    //dataset.open("test.txt", std::ios::out|std::ios::binary);

    int64_t n_contract = 0;
    int64_t n_function = 0;
    int64_t n_patch = 0;
    int64_t n_malicious = 0;
    int64_t n_benign = 0;
    // label
    int counter = 0;
    bool flag = false;
    while (contract_list >> file_name) {
        counter ++;
        // if (counter > 44000) break;
        // if (!flag) {
        //     if (counter >= 40001) {
        //         flag = true;
        //     } else {
        //         continue;
        //     }
        // }
        // file_name = "0x62edb11263cd775d549a9d9e38980014dbbfdedd.sol";
        std::cout << file_name << std::endl;
        std::string file_prefix = file_name.substr(0, file_name.find("."));
        // parse program
        std::string input = solver::Util::ReadFileIntoString(solver::contract_dir + file_name);
        solver::Parser* parser = new solver::Parser(new solver::Lexer(input));
        parser->ParseProgram();
#ifdef DEBUG
        // parser->PrintTokenFlow();
#endif
        n_contract += parser->total_contract_num_;
        n_function += parser->total_function_num_;
        std::vector<std::vector<solver::Token*>> code_patch = parser->code_patches;
        n_patch += code_patch.size();
        // for (int i = 0;i<code_patch.size();++i)
        //     for (int j = 0;j<code_patch[i].size();++j)
        //         std::cout<<code_patch[i][j]->GetLiteral()<< std::endl;
        // break;
        // label program patch
        solver::Labeler* labeler = new solver::Labeler(file_prefix);
        // labeler->ReadFile();
        std::ofstream test("./sbcurated/ul/"+file_prefix+".txt", std::ios_base::app);
        bool result = labeler->AddLabel(file_name, code_patch, test);
        test.close();
        // if (result) {
        //     test_list << file_prefix << "  1\n";
        // } else {
        //     test_list << file_prefix << "  0\n";
        // }
        n_benign += labeler->n_benign_;
        n_malicious += labeler->n_malicious_;
        // log
        std::cout << "n_file = " << counter
                  << ", n_contract = " << n_contract
                  << ", n_function = " << n_function
                  << ", n_patch = " << n_patch
                  << ", n_malicious = " << n_malicious
                  << ", n_benign = " << n_benign << std::endl;
        // break;
        // if (n_contract > 10000) break;
    }
    contract_list.close();
    // dataset.close();
    return 0;
}