#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"

namespace svc {

class VarStateNode : public Node {
public:
    VarStateNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "var_statement";
    }
    ~VarStateNode() = default;

    void Parse() override {
    }
};

} // namespace svc 