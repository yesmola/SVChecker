#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <set>

#include "token.h"

namespace solver {
class Labeler {
  public:
    explicit Labeler(std::string file_name) { 
      file_name_ = file_name; 
      // n_iu_ = 0;
      // n_io_ = 0;
      // n_td_ = 0;
      // n_re_ = 0;
      // n_cd_ = 0;
      n_malicious_ = 0;
      n_benign_ = 0;
    }
    ~Labeler() {}
    void ReadFile();
    bool AddLabel(std::string& source, std::vector<std::vector<Token*>>& code_patches, std::ofstream& ofile); 

  public:
    // int64_t n_iu_;
    // int64_t n_io_;
    // int64_t n_td_;
    // int64_t n_re_;
    // int64_t n_cd_;
    int64_t n_malicious_;
    int64_t n_benign_;

  private:
    std::string file_name_;
    std::set<int64_t> error_lines_;
};

} // namespace solver