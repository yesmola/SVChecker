#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"
#include "ast/function_node.h"
#include "ast/variable_node.h"
#include "ast/modifier_node.h"

namespace svc {

class ContractNode : public Node {
public:
    ContractNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "contract";
    }
    ~ContractNode() = default;

    void Parse() override {
        for (auto& child : node_["children"]) {
            if (child["name"] == "FunctionDefinition" && child["attributes"]["name"] != "") {
                auto func_node = std::make_shared<FunctionNode>(child, source_code_);
                func_node->Parse();
                children_.push_back(func_node);
            }
            if (child["name"] == "VariableDeclaration" && child["attributes"]["name"] != "") {
                auto var_node = std::make_shared<VariableNode>(child, source_code_);
                var_node->Parse();
                children_.push_back(var_node);
            }
            if (child["name"] == "ModifierDefinition" && child["attributes"]["name"] != "") {
                auto modifier_node = std::make_shared<ModifierNode>(child, source_code_);
                modifier_node->Parse();
                children_.push_back(modifier_node);
            }
        }
        for (auto& node : children_) {
            for (auto& child : node->GetChildren()) {
                child->SetFather(node);
            }
        }
        return;
    }
};

} // namespace svc 