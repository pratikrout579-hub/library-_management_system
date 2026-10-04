#include "utils/ActivityLogger.h"

#include <fcntl.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace library {
namespace {
// Sends "EVENT|message" to the Linux character driver (/dev/library_device).
// Best effort: if the kernel module is not loaded the web app keeps working.
void sendToDevice(const std::string& event, const std::string& message) noexcept {
    try {
        const std::string name = (event == "BOOK_DELETED") ? "BOOK_REMOVED" : event;
        const std::string line = name + "|" + message;
        const int fd = open("/dev/library_device", O_WRONLY | O_NONBLOCK);
        if (fd < 0) return;
        const ssize_t written = ::write(fd, line.c_str(), line.size());
        (void)written;
        close(fd);
    } catch (...) {}
}
} // namespace
ActivityLogger::ActivityLogger(std::string logPath) : logPath_(std::move(logPath)) {}
void ActivityLogger::write(const std::string& event, const std::string& message) const noexcept {
    try {
        std::filesystem::create_directories(std::filesystem::path(logPath_).parent_path());
        std::ofstream output(logPath_, std::ios::app);
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        output << std::put_time(std::localtime(&now), "%F %T") << " | " << event << " | " << message << '\n';
    } catch (...) {
        // Logging must never interrupt the library workflow.
    }
    sendToDevice(event, message);
}
} // namespace library
