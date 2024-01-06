#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

#include <spdlog/spdlog.h>

namespace svc {

class FileUtils {
public:
    // Read file content to string
    static void ReadFileToString(const std::string& path, std::string& data) {
        std::ifstream open_file(path);
        std::ostringstream buf;
        char ch;
        while (buf && open_file.get(ch)) {
            buf.put(ch);
        }
        data = buf.str();
    }

    // Write string to tmp file
    static void WriteStringToFile(const std::string& path, std::string& data) {
        std::ofstream open_file(path);
        open_file << data;
        open_file.close();
    }

    // Create a directory
    static void CreateDir(const std::string& path) {
        std::filesystem::create_directories(path);
    }

    // Check wheather file exist
    static bool IsFileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
    }

    // Get all files in the directory
    static void ListFiles(const std::string& dir, std::vector<std::string>& files, const std::string& filter) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                if (filter.empty() || entry.path().extension() == filter) {
                    files.push_back(entry.path().string());
                }
            }
        }
    }
};

} // namespace svc