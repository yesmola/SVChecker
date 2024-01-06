#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"

namespace svc {

class ModifierNode : public Node {
public:
    ModifierNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "modifier";
    }
    ~ModifierNode() = default;

    void Parse() override {
    }
};

} // namespace svc 