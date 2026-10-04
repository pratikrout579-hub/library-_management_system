#include "services/AuthService.h"
#include "utils/ActivityLogger.h"

#include <chrono>
#include <iomanip>
#include <openssl/sha.h>
#include <random>
#include <sstream>
#include <stdexcept>

namespace library {
AuthService::AuthService(AuthRepository& repository, ActivityRepository& activities, ActivityLogger& logger)
    : repository_(repository), activities_(activities), logger_(logger) {}

std::string AuthService::hashPassword(const std::string& password) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.data()), password.size(), digest);
    std::ostringstream value;
    for (unsigned char byte : digest) value << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return value.str();
}

std::string AuthService::login(const std::string& email, const std::string& password) {
    const auto person = repository_.findByEmail(email);
    if (!person || person->at("status") != "ACTIVE" || person->at("password_hash") != hashPassword(password))
        throw std::runtime_error("Invalid email or password");
    std::random_device device;
    std::mt19937_64 engine(device());
    const auto token = std::to_string(engine()) + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    SessionUser user{std::stoi(person->at("id")), person->at("role"), person->at("name"), person->at("email")};
    sessions_[token] = user;
    activities_.create(user.role, user.id, "LOGIN", user.name + " signed in");
    logger_.write("LOGIN", user.role + ": " + user.email);
    return token;
}
void AuthService::logout(const std::string& token) { sessions_.erase(token); }
std::optional<SessionUser> AuthService::userFromToken(const std::string& token) const {
    const auto found=sessions_.find(token); return found==sessions_.end()?std::nullopt:std::optional<SessionUser>{found->second};
}
} // namespace library
