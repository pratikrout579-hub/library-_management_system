#pragma once

#include <string>

namespace library {
class ActivityLogger {
public:
    explicit ActivityLogger(std::string logPath);
    void write(const std::string& event, const std::string& message) const noexcept;
private: std::string logPath_;
};
} // namespace library
