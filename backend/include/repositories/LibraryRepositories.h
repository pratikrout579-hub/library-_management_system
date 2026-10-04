#pragma once

#include <optional>
#include <string>
#include <vector>
#include "repositories/Database.h"

namespace library {
class AuthRepository {
public:
    explicit AuthRepository(Database& db) : db_(db) {}
    std::optional<Row> findByEmail(const std::string& email) const;
private: Database& db_;
};

class BookRepository {
public:
    explicit BookRepository(Database& db) : db_(db) {}
    std::vector<Row> list(const std::string& search, const std::string& category, const std::string& availability, bool includeInactive = false) const;
    std::optional<Row> find(int id) const;
    int create(const BookInput& input);
    void update(int id, const BookInput& input);
    void deactivate(int id);
    bool adjustAvailability(int id, int difference);
private: Database& db_;
};

class StudentRepository {
public:
    explicit StudentRepository(Database& db) : db_(db) {}
    std::vector<Row> list(const std::string& search, bool includeInactive = false) const;
    std::optional<Row> find(int id) const;
    int create(const StudentInput& input, const std::string& passwordHash);
    void update(int id, const StudentInput& input);
    void setStatus(int id, const std::string& status);
private: Database& db_;
};

class RequestRepository {
public:
    explicit RequestRepository(Database& db) : db_(db) {}
    bool hasActiveRequest(int studentId, int bookId) const;
    int create(int studentId, int bookId);
    std::optional<Row> find(int id) const;
    std::vector<Row> listAdmin(const std::string& status) const;
    std::vector<Row> listStudent(int studentId) const;
    void approve(int id, int adminId);
    void reject(int id, int adminId, const std::string& reason);
private: Database& db_;
};

class IssueRepository {
public:
    explicit IssueRepository(Database& db) : db_(db) {}
    bool hasActiveIssue(int studentId, int bookId) const;
    int create(int studentId, int bookId, int adminId, const std::string& issueDate, const std::string& dueDate, const std::string& requestId = "");
    std::optional<Row> find(int id) const;
    std::vector<Row> listAdmin(const std::string& status) const;
    std::vector<Row> listStudent(int studentId, bool history) const;
    void returnBook(int id, const std::string& returnDate, double fine);
private: Database& db_;
};

class ActivityRepository {
public:
    explicit ActivityRepository(Database& db) : db_(db) {}
    void create(const std::string& actorType, int actorId, const std::string& event, const std::string& message);
    std::vector<Row> recent(int limit = 10) const;
private: Database& db_;
};

class ReportRepository {
public:
    explicit ReportRepository(Database& db) : db_(db) {}
    Row summary() const;
private: Database& db_;
};
} // namespace library
