#pragma once

#include <sqlite3.h>
#include <optional>
#include <string>
#include <vector>
#include "models/Entities.h"

namespace library {
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void execute(const std::string& sql, const std::vector<std::string>& values = {});
    std::vector<Row> query(const std::string& sql, const std::vector<std::string>& values = {}) const;
    std::optional<Row> one(const std::string& sql, const std::vector<std::string>& values = {}) const;
    int changes() const;
    long long lastInsertId() const;
    void executeScript(const std::string& path);
    void begin();
    void commit();
    void rollback() noexcept;
private:
    sqlite3* connection_{};
    sqlite3_stmt* prepare(const std::string& sql, const std::vector<std::string>& values) const;
};
} // namespace library
