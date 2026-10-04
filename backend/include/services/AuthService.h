#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include "models/Entities.h"
#include "repositories/LibraryRepositories.h"

namespace library {
class ActivityLogger;
class AuthService {
public:
    AuthService(AuthRepository& repository, ActivityRepository& activities, ActivityLogger& logger);
    std::string login(const std::string& email, const std::string& password);
    void logout(const std::string& token);
    std::optional<SessionUser> userFromToken(const std::string& token) const;
    static std::string hashPassword(const std::string& password);
private:
    AuthRepository& repository_;
    ActivityRepository& activities_;
    ActivityLogger& logger_;
    std::unordered_map<std::string, SessionUser> sessions_;
};
} // namespace library
