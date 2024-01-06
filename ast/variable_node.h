#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"

namespace svc {

class VariableNode : public Node {
public:
    VariableNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "variable";
    }
    ~VariableNode() = default;

    void Parse() override {
    }
};

} // namespace svc 