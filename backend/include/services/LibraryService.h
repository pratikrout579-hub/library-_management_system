#pragma once

#include "models/Entities.h"
#include "repositories/LibraryRepositories.h"

namespace library {
class ActivityLogger;
class LibraryService {
public:
    LibraryService(Database& db, BookRepository& books, StudentRepository& students, RequestRepository& requests,
                   IssueRepository& issues, ActivityRepository& activities, ActivityLogger& logger);
    int addBook(const BookInput& input, int adminId);
    void updateBook(int id, const BookInput& input, int adminId);
    void deactivateBook(int id, int adminId);
    int addStudent(const StudentInput& input, int adminId);
    void updateStudent(int id, const StudentInput& input, int adminId);
    void setStudentStatus(int id, const std::string& status, int adminId);
    int requestBook(int studentId, int bookId);
    void approveRequest(int requestId, int adminId, const std::string& dueDate);
    void rejectRequest(int requestId, int adminId, const std::string& reason);
    int directIssue(int studentId, int bookId, int adminId, const std::string& issueDate, const std::string& dueDate);
    double returnIssue(int issueId, int adminId, const std::string& returnDate);
private:
    Database& db_; BookRepository& books_; StudentRepository& students_; RequestRepository& requests_; IssueRepository& issues_; ActivityRepository& activities_; ActivityLogger& logger_;
    void validateBook(const BookInput& input) const;
    void validateStudent(const StudentInput& input, bool needsPassword) const;
    static double calculateFine(const std::string& dueDate, const std::string& returnDate);
};
} // namespace library
