#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <unordered_map>

#include <json/json.hpp>

#include "ast/common.h"

namespace svc{

class Node {
public:
    Node(nlohmann::json node, std::string& source_code) {
        node_ = node;
        source_code_ = source_code;
        type_ = "unknown";
        name_ = node_.at("attributes").contains("name") ? node_["attributes"]["name"] : "";
        part_code_ = source_code_.substr(ast::GetStartPosAndLen(node_["src"]).first, ast::GetStartPosAndLen(node_["src"]).second);
    }
    virtual ~Node() {
        children_.clear();
    };

    virtual void Parse() = 0;

    std::string GetName() { return name_; }
    std::string GetType() { return type_; }
    void SetFather(std::shared_ptr<Node> father) {
        father_ = father;
    }
    std::shared_ptr<Node> GetFather() { return father_.lock(); }
    std::vector<std::shared_ptr<Node>> GetChildren() { return children_; }

    std::string GetPartCode() { return part_code_; }
    nlohmann::json GetJsonNode() { return node_; }
    std::string GetBlockCode() { return block_code_; }

    std::set<int> GetIds() { return ids_; }
protected:
    void ParseIds() {
        std::queue<nlohmann::json> q;
        q.push(node_);
        while (!q.empty()) {
            nlohmann::json node = q.front();
            q.pop();
            if (node.contains("id") && node["id"] != nullptr) {
                int id = node["id"];
                ids_.insert(id);
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
    nlohmann::json node_;
    std::string name_;
    std::string type_;
    std::string source_code_;
    std::string part_code_;
    std::string block_code_; // only for func node
    std::set<int> ids_;

    std::weak_ptr<Node> father_;
    std::vector<std::shared_ptr<Node>> children_;
};

} // namespace svc
