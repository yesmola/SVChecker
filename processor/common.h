#pragma once

#include <vector>
#include <string>
#include <set>

// #define DEBUG 1

#define SUCCESS 0
// ERROR CODE
#define SWITCH_VERSION_ERROR 1
#define COMPILE_ERROR 2
#define AST_JSON_ERROR 3
#define IOE_INJECT_ERROR 4
#define ACE_INJECT_ERROR 5
#define TS_INJECT_ERROR 6
#define ULC_INJECT_ERROR 7
#define DOS_INJECT_ERROR 8
#define RE_INJECT_ERROR 9

namespace svc {

// origin dataset path
const std::string kDataSet4 = "/home/yuanye/graduation/Dataset/4/";
const std::string kDataSet5 = "/home/yuanye/graduation/Dataset/5/";
const std::string kDataSet6 = "/home/yuanye/graduation/Dataset/6/";
const std::string kDataSetWild = "/home/yuanye/graduation/Dataset/wild/";

const std::string kTmpDataset = "/home/yuanye/graduation/SVChecker/tmp-dataset/";

// dst dataset path
const std::string kDstDatasetPath = "/home/yuanye/graduation/BugDataset_new/";

// support compile version from 0.5.0 to 0.7.3
const std::set<std::string> kSupportedVersions = {"0.5.0", "0.5.1", "0.5.2", "0.5.3",
                                                "0.5.4", "0.5.5", "0.5.6", "0.5.7",
                                                "0.5.8", "0.5.9", "0.5.10", "0.5.11",
                                                "0.5.12", "0.5.13", "0.5.14", "0.5.15",
                                                "0.5.16", "0.5.17", "0.5.24", "0.6.0", "0.6.1", 
                                                "0.6.2", "0.6.3", "0.6.4", "0.6.5",
                                                "0.6.6", "0.6.7", "0.6.8", "0.6.9",
                                                "0.6.10", "0.6.11", "0.6.12", "0.7.0",
                                                "0.7.1", "0.7.2", "0.4.25"};

// tmp file path
const std::string kTmpPath = "./tmp/svc";
// tmp file
const std::string kTmpFile = "./tmp/svc/tmp.sol";
// tmp json ast file
const std::string kTmpJsonFile = "./tmp/svc/tmp.sol_json.ast";

// 定义的类型
enum class DefType : uint {
    UNKNOWN = 0,
    CONTRACT_DEF = 1,
    FUNC_DEF = 2,
    STRUCT_DEF = 3,
    VAR_DEF = 4
};

} // namespace svc