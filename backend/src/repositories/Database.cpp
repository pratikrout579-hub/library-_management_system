#include "repositories/Database.h"

#include <fstream>
#include <stdexcept>

namespace library {
Database::Database(const std::string& path) {
    if (sqlite3_open_v2(path.c_str(), &connection_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::string message = connection_ ? sqlite3_errmsg(connection_) : "Unable to open SQLite database";
        if (connection_) sqlite3_close(connection_);
        throw std::runtime_error(message);
    }
    execute("PRAGMA foreign_keys = ON");
    execute("PRAGMA busy_timeout = 5000");
}

Database::~Database() { if (connection_) sqlite3_close(connection_); }

sqlite3_stmt* Database::prepare(const std::string& sql, const std::vector<std::string>& values) const {
    sqlite3_stmt* statement{};
    if (sqlite3_prepare_v2(connection_, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(connection_));
    for (size_t index = 0; index < values.size(); ++index) {
        if (sqlite3_bind_text(statement, static_cast<int>(index + 1), values[index].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(statement);
            throw std::runtime_error(sqlite3_errmsg(connection_));
        }
    }
    return statement;
}

void Database::execute(const std::string& sql, const std::vector<std::string>& values) {
    sqlite3_stmt* statement = prepare(sql, values);
    int result;
    do {
        result = sqlite3_step(statement);
    } while (result == SQLITE_ROW);
    if (result != SQLITE_DONE) {
        std::string message = sqlite3_errmsg(connection_);
        sqlite3_finalize(statement);
        throw std::runtime_error(message);
    }
    sqlite3_finalize(statement);
}

std::vector<Row> Database::query(const std::string& sql, const std::vector<std::string>& values) const {
    sqlite3_stmt* statement = prepare(sql, values);
    std::vector<Row> rows;
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Row row;
        for (int index = 0; index < sqlite3_column_count(statement); ++index) {
            const auto* text = sqlite3_column_text(statement, index);
            row[sqlite3_column_name(statement, index)] = text ? reinterpret_cast<const char*>(text) : "";
        }
        rows.push_back(std::move(row));
    }
    const int result = sqlite3_errcode(connection_);
    sqlite3_finalize(statement);
    if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) throw std::runtime_error(sqlite3_errmsg(connection_));
    return rows;
}

std::optional<Row> Database::one(const std::string& sql, const std::vector<std::string>& values) const {
    auto rows = query(sql, values);
    return rows.empty() ? std::nullopt : std::optional<Row>{rows.front()};
}

int Database::changes() const { return sqlite3_changes(connection_); }
long long Database::lastInsertId() const { return sqlite3_last_insert_rowid(connection_); }

void Database::executeScript(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot read SQL script: " + path);
    const std::string script((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    char* error{};
    if (sqlite3_exec(connection_, script.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : "Unable to execute SQL script";
        sqlite3_free(error);
        throw std::runtime_error(message);
    }
}
void Database::begin() { execute("BEGIN IMMEDIATE"); }
void Database::commit() { execute("COMMIT"); }
void Database::rollback() noexcept { try { execute("ROLLBACK"); } catch (...) {} }
} // namespace library
