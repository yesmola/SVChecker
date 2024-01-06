#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <iostream>

#include <json/json.hpp>

#include "ast/common.h"
#include "ast/node.h"
#include "ast/contract_node.h"

namespace svc {

class AstWrapper {
public:
    explicit AstWrapper(nlohmann::json root, std::string& source_code) {
        ast_json_ = root;
        source_code_ = source_code;
    }
    ~AstWrapper() {
        contract_nodes_.clear();
    };

    void Parse() {
        std::vector<nlohmann::json> contract_def = ast::GetNodesByName(ast_json_, "name", "ContractDefinition");
        for (size_t i = 0; i < contract_def.size(); i ++) {
            if (contract_def[i]["attributes"]["contractKind"] != "contract") {
                continue;
            }
            std::shared_ptr<ContractNode> contract_node = std::make_shared<ContractNode>(contract_def[i], source_code_);
            contract_node->Parse();
            contract_nodes_.push_back(contract_node);
        }

        for (auto& node : contract_nodes_) {
            for (auto& child : node->GetChildren()) {
                child->SetFather(node);
            }
        }
        // 解析id和数字
        std::queue<nlohmann::json> q;
        q.push(ast_json_);
        while (!q.empty()) {
            nlohmann::json node = q.front();
            q.pop();
            if (node.contains("id") && node["id"] != nullptr && node.contains("name") && node.contains("attributes") && node.at("attributes").contains("name") && 
                node["attributes"]["name"] != nullptr && node["attributes"]["name"] != "") {
                std::string define_name = node["attributes"]["name"];
                std::string define_type = node["name"];
                int id = node["id"];
                std::string src = node["src"];
                if (define_type == "StructDefinition" ||
                    define_type == "VariableDeclaration") {
                    id_src_map_[id] = src;
                }
            }
                
            if (node.contains("attributes") && node["attributes"].contains("token") && 
                node["attributes"].contains("value") && node["attributes"]["value"] != nullptr && node["attributes"]["value"] != "") {
                std::string number = node["attributes"]["value"];
                all_num_.insert(number);
            } 
            for (auto& iter : node.items()) {
                if (iter.key() == "children" && iter.value().is_array()) {
                    for (auto& child : iter.value()) {
                        q.push(child);
                    }
                }
            }
        }
    }

    std::string GetSourceCode() { return source_code_; }
    nlohmann::json GetJsonNode() { return ast_json_; }
    std::vector<std::shared_ptr<Node>> GetNodes() { return contract_nodes_; }

    std::unordered_map<int, std::string> GetAllIdSrc() {
        return id_src_map_;
    }

    std::set<std::string> GetAllNumber() {
        return all_num_;
    }
private:
    nlohmann::json ast_json_;
    std::string source_code_;

    std::vector<std::shared_ptr<Node>> contract_nodes_;

    std::set<std::string> all_num_;
    std::unordered_map<int, std::string> id_src_map_;
};

} // namespace svc