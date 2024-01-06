#pragma once

#include <vector>

#include <json/json.hpp>

namespace svc {

namespace ast {

std::vector<nlohmann::json> GetNodesByName(nlohmann::json& root, const std::string& key, const std::string& value) {
    std::vector<nlohmann::json> nodes;
    std::queue<nlohmann::json> q;
    q.push(root);
    while (!q.empty()) {
        nlohmann::json node = q.front();
        q.pop();
        for (auto& iter : node.items()) {
            if (iter.key() == key && iter.value().is_string() && iter.value() == value) {
                nodes.push_back(node);
            }
            if (iter.key() == "children" && iter.value().is_array()) {
                for (auto& child : iter.value()) {
                    q.push(child);
                }
            }
        }
    }
    return nodes;
}

std::pair<int, int> GetStartPosAndLen(std::string src) {
        std::vector<std::string> info = StringUtils::SplitString(src, ':');
        int start_pos = std::stoi(info[0]);
        int length = std::stoi(info[1]);
        return std::make_pair(start_pos,  length);
    }
    
} // namespace ast
} // namespace svc