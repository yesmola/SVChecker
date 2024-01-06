#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"

namespace svc {

class AssginmentNode : public Node {
public:
    AssginmentNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "assignment";
    }
    ~AssginmentNode() = default;

    void Parse() override {
    }
};

} // namespace svc 