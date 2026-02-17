#include "Logger.h"
#include <iostream> // cout を使うなら必要

namespace Logger {
    void Log(const std::string& message) {
        std::cout << message << std::endl;
    }
}
