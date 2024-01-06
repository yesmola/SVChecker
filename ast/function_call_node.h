#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"

namespace svc {

class FuncCallNode : public Node {
public:
    FuncCallNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "function_call";
    }
    ~FuncCallNode() = default;

    void Parse() override {
    }
};

} // namespace svc 