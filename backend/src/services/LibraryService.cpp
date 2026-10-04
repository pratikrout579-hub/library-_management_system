#include "services/LibraryService.h"
#include "services/AuthService.h"
#include "utils/ActivityLogger.h"

#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace library {
namespace {
std::string today() {
    const auto now=std::time(nullptr); std::tm current{};
    localtime_r(&now,&current);
    std::ostringstream date; date << std::put_time(&current,"%F"); return date.str();
}
}
LibraryService::LibraryService(Database& db, BookRepository& books, StudentRepository& students, RequestRepository& requests,
                               IssueRepository& issues, ActivityRepository& activities, ActivityLogger& logger)
    : db_(db), books_(books), students_(students), requests_(requests), issues_(issues), activities_(activities), logger_(logger) {}

void LibraryService::validateBook(const BookInput& input) const {
    if (input.title.empty() || input.author.empty() || input.category.empty() || input.isbn.empty()) throw std::runtime_error("Title, author, category and ISBN are required");
    if (!std::regex_match(input.isbn, std::regex("[0-9Xx-]{10,17}"))) throw std::runtime_error("Enter a valid ISBN");
    if (input.totalQuantity < 1) throw std::runtime_error("Total quantity must be positive");
    if (input.publicationYear < 1000 || input.publicationYear > 2100) throw std::runtime_error("Enter a valid publication year");
}
void LibraryService::validateStudent(const StudentInput& input, bool needsPassword) const {
    if (input.studentCode.empty() || input.name.empty() || input.department.empty() || input.course.empty()) throw std::runtime_error("Student details are incomplete");
    if (!std::regex_match(input.email, std::regex("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"))) throw std::runtime_error("Enter a valid email");
    if (input.studyYear < 1 || input.studyYear > 8) throw std::runtime_error("Study year must be between 1 and 8");
    if (needsPassword && input.password.size() < 8) throw std::runtime_error("Initial password must have at least 8 characters");
}
int LibraryService::addBook(const BookInput& input, int adminId) { validateBook(input); auto id=books_.create(input); activities_.create("ADMIN",adminId,"BOOK_ADDED","Added book: "+input.title); logger_.write("BOOK_ADDED",input.title); return id; }
void LibraryService::updateBook(int id, const BookInput& input, int adminId) { validateBook(input); books_.update(id,input); activities_.create("ADMIN",adminId,"BOOK_UPDATED","Updated book: "+input.title); logger_.write("BOOK_UPDATED",input.title); }
void LibraryService::deactivateBook(int id, int adminId) { auto book=books_.find(id); if(!book) throw std::runtime_error("Book not found"); books_.deactivate(id); activities_.create("ADMIN",adminId,"BOOK_DELETED","Deactivated book: "+book->at("title")); logger_.write("BOOK_DELETED",book->at("title")); }
int LibraryService::addStudent(const StudentInput& input, int adminId) { validateStudent(input,true); auto id=students_.create(input,AuthService::hashPassword(input.password)); activities_.create("ADMIN",adminId,"STUDENT_ADDED","Added student: "+input.name); logger_.write("STUDENT_ADDED",input.name); return id; }
void LibraryService::updateStudent(int id, const StudentInput& input, int adminId) { validateStudent(input,false); students_.update(id,input); activities_.create("ADMIN",adminId,"STUDENT_UPDATED","Updated student: "+input.name); }
void LibraryService::setStudentStatus(int id, const std::string& status, int adminId) { if(status!="ACTIVE" && status!="INACTIVE") throw std::runtime_error("Status must be ACTIVE or INACTIVE"); students_.setStatus(id,status); activities_.create("ADMIN",adminId,"STUDENT_STATUS_UPDATED","Student status set to "+status); }

int LibraryService::requestBook(int studentId, int bookId) {
    const auto student=students_.find(studentId); if(!student || student->at("status")!="ACTIVE") throw std::runtime_error("Only active students can request books");
    const auto book=books_.find(bookId); if(!book || book->at("status")!="ACTIVE") throw std::runtime_error("This book is unavailable");
    if(issues_.hasActiveIssue(studentId,bookId)) throw std::runtime_error("You already have this book issued");
    if(requests_.hasActiveRequest(studentId,bookId)) throw std::runtime_error("You already have a pending request for this book");
    auto id=requests_.create(studentId,bookId); activities_.create("STUDENT",studentId,"BOOK_REQUESTED","Requested: "+book->at("title")); logger_.write("BOOK_REQUESTED",student->at("email")+" | "+book->at("title")); return id;
}
void LibraryService::approveRequest(int requestId, int adminId, const std::string& dueDate) {
    const auto request=requests_.find(requestId); if(!request || request->at("status")!="PENDING") throw std::runtime_error("Request is no longer pending");
    const int studentId=std::stoi(request->at("student_id")), bookId=std::stoi(request->at("book_id"));
    const auto student=students_.find(studentId); const auto book=books_.find(bookId);
    if(!student || student->at("status")!="ACTIVE") throw std::runtime_error("Student is not active");
    if(!book || book->at("status")!="ACTIVE") throw std::runtime_error("Book is not active");
    if(issues_.hasActiveIssue(studentId,bookId)) throw std::runtime_error("Student already has an active issue for this book");
    if(dueDate.empty()) throw std::runtime_error("Due date is required");
    db_.begin();
    try { if(!books_.adjustAvailability(bookId,-1)) throw std::runtime_error("No available copies remain"); issues_.create(studentId,bookId,adminId,today(),dueDate,std::to_string(requestId)); requests_.approve(requestId,adminId); activities_.create("ADMIN",adminId,"REQUEST_APPROVED","Approved request for: "+book->at("title")); db_.commit(); logger_.write("REQUEST_APPROVED",book->at("title")); }
    catch(...) { db_.rollback(); throw; }
}
void LibraryService::rejectRequest(int requestId, int adminId, const std::string& reason) { const auto request=requests_.find(requestId); if(!request || request->at("status")!="PENDING") throw std::runtime_error("Request is no longer pending"); requests_.reject(requestId,adminId,reason); activities_.create("ADMIN",adminId,"REQUEST_REJECTED","Rejected request #"+std::to_string(requestId)); logger_.write("REQUEST_REJECTED",std::to_string(requestId)); }
int LibraryService::directIssue(int studentId, int bookId, int adminId, const std::string& issueDate, const std::string& dueDate) {
    const auto student=students_.find(studentId); const auto book=books_.find(bookId); if(!student || student->at("status")!="ACTIVE") throw std::runtime_error("Student is not active"); if(!book || book->at("status")!="ACTIVE") throw std::runtime_error("Book is not active"); if(issueDate.empty() || dueDate.empty() || dueDate<issueDate) throw std::runtime_error("Enter valid issue and due dates"); if(issues_.hasActiveIssue(studentId,bookId)) throw std::runtime_error("Student already has this book");
    db_.begin(); try { if(!books_.adjustAvailability(bookId,-1)) throw std::runtime_error("No available copies remain"); auto id=issues_.create(studentId,bookId,adminId,issueDate,dueDate); activities_.create("ADMIN",adminId,"BOOK_ISSUED","Issued "+book->at("title")+" to "+student->at("name")); db_.commit(); logger_.write("BOOK_ISSUED",book->at("title")); return id; } catch(...) { db_.rollback(); throw; }
}
double LibraryService::calculateFine(const std::string& dueDate, const std::string& returnDate) { std::tm due{}, returned{}; std::istringstream dueStream(dueDate), returnedStream(returnDate); if(!(dueStream>>std::get_time(&due,"%Y-%m-%d")) || !(returnedStream>>std::get_time(&returned,"%Y-%m-%d"))) throw std::runtime_error("Dates must use YYYY-MM-DD"); auto days=std::difftime(std::mktime(&returned),std::mktime(&due))/86400; return days>0?days*5.0:0.0; }
double LibraryService::returnIssue(int issueId, int adminId, const std::string& returnDate) { const auto issue=issues_.find(issueId); if(!issue) throw std::runtime_error("Issue not found"); if(issue->at("status")!="ISSUED") throw std::runtime_error("This book has already been returned"); if(returnDate.empty()) throw std::runtime_error("Return date is required"); const auto fine=calculateFine(issue->at("due_date"),returnDate); db_.begin(); try { issues_.returnBook(issueId,returnDate,fine); if(!books_.adjustAvailability(std::stoi(issue->at("book_id")),1)) throw std::runtime_error("Cannot update available quantity"); activities_.create("ADMIN",adminId,"BOOK_RETURNED","Returned: "+issue->at("book_title")); db_.commit(); logger_.write("BOOK_RETURNED",issue->at("book_title")); return fine; } catch(...) { db_.rollback(); throw; } }
} // namespace library
