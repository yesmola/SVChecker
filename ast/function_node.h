#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"
#include "ast/variable_node.h"
#include "ast/assignment_node.h"
#include "ast/var_state_node.h"
#include "ast/function_call_node.h"

namespace svc {

class FunctionNode : public Node {
public:
    FunctionNode(nlohmann::json node, std::string& source_code) : Node(node, source_code) {
        type_ = "function";
    }
    ~FunctionNode() = default;

    void Parse() override {
        std::vector<nlohmann::json> var_nodes = ast::GetNodesByName(node_, "name", "VariableDeclaration");
        for (auto& node : var_nodes) {
            if (node["attributes"]["name"] != "") {
                auto var_node = std::make_shared<VariableNode>(node, source_code_);
                var_node->Parse();
                children_.push_back(var_node);
            }
        }
        std::vector<nlohmann::json> assign_nodes = ast::GetNodesByName(node_, "name", "Assignment");
        for (auto& node : assign_nodes) {
            auto assgin_node = std::make_shared<AssginmentNode>(node, source_code_);
            assgin_node->Parse();
            children_.push_back(assgin_node);
        }
        std::vector<nlohmann::json> var_state_nodes = ast::GetNodesByName(node_, "name", "VariableDeclarationStatement");
        for (auto& node : var_state_nodes) {
            auto var_state_node = std::make_shared<VarStateNode>(node, source_code_);
            var_state_node->Parse();
            children_.push_back(var_state_node);
        }
        std::vector<nlohmann::json> func_call_nodes = ast::GetNodesByName(node_, "name", "FunctionCall");
        for (auto& node : func_call_nodes) {
            auto func_call_node = std::make_shared<FuncCallNode>(node, source_code_);
            func_call_node->Parse();
            children_.push_back(func_call_node);
        }
        if (node_["children"].is_array()) {
            for (size_t i = 0; i < node_["children"].size(); i++) {
                auto& child = node_["children"][i];
                if (child["name"] == "Block") {
                    block_code_ = source_code_.substr(ast::GetStartPosAndLen(child["src"]).first, ast::GetStartPosAndLen(child["src"]).second);
                }
            }
        }
        ParseIds();
    }
private:
    std::vector<Node> assginment_nodes_; // a = b;
    std::vector<Node> var_state_node_; // uint c = a + b;
};

} // namespace svc 