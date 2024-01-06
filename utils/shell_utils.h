#pragma once

#include <string>

namespace svc {

class ShellUtils {
public:
    static int RunCommand(const std::string &cmd) {
        int ret = system(cmd.c_str());
        return ret;
    }
};

} // namespace svc