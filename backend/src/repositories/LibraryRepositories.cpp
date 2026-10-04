#include "repositories/LibraryRepositories.h"

#include <stdexcept>

namespace library {
std::optional<Row> AuthRepository::findByEmail(const std::string& email) const {
    return db_.one("SELECT id,name,email,password_hash,'ADMIN' AS role,status FROM admins WHERE email=? "
                   "UNION ALL SELECT id,name,email,password_hash,'STUDENT' AS role,status FROM students WHERE email=? LIMIT 1", {email, email});
}

std::vector<Row> BookRepository::list(const std::string& search, const std::string& category, const std::string& availability, bool includeInactive) const {
    std::string sql = "SELECT * FROM books WHERE 1=1";
    std::vector<std::string> values;
    if (!includeInactive) sql += " AND status='ACTIVE'";
    if (!search.empty()) { sql += " AND (title LIKE ? OR author LIKE ? OR isbn LIKE ?)"; const auto like="%"+search+"%"; values.insert(values.end(), {like,like,like}); }
    if (!category.empty()) { sql += " AND category=?"; values.push_back(category); }
    if (availability == "AVAILABLE") sql += " AND available_quantity > 0";
    if (availability == "UNAVAILABLE") sql += " AND available_quantity = 0";
    sql += " ORDER BY title";
    return db_.query(sql, values);
}
std::optional<Row> BookRepository::find(int id) const { return db_.one("SELECT * FROM books WHERE id=?", {std::to_string(id)}); }
int BookRepository::create(const BookInput& input) {
    db_.execute("INSERT INTO books(title,author,category,isbn,publisher,publication_year,total_quantity,available_quantity) VALUES(?,?,?,?,?,?,?,?)",
        {input.title,input.author,input.category,input.isbn,input.publisher,std::to_string(input.publicationYear),std::to_string(input.totalQuantity),std::to_string(input.totalQuantity)});
    return static_cast<int>(db_.lastInsertId());
}
void BookRepository::update(int id, const BookInput& input) {
    const auto current = find(id); if (!current) throw std::runtime_error("Book not found");
    const int issued = std::stoi(current->at("total_quantity")) - std::stoi(current->at("available_quantity"));
    if (input.totalQuantity < issued) throw std::runtime_error("Total quantity cannot be lower than currently issued copies");
    db_.execute("UPDATE books SET title=?,author=?,category=?,isbn=?,publisher=?,publication_year=?,total_quantity=?,available_quantity=?,updated_at=CURRENT_TIMESTAMP WHERE id=?",
        {input.title,input.author,input.category,input.isbn,input.publisher,std::to_string(input.publicationYear),std::to_string(input.totalQuantity),std::to_string(input.totalQuantity-issued),std::to_string(id)});
    if (!db_.changes()) throw std::runtime_error("Book not found");
}
void BookRepository::deactivate(int id) { db_.execute("UPDATE books SET status='INACTIVE',updated_at=CURRENT_TIMESTAMP WHERE id=?", {std::to_string(id)}); if(!db_.changes()) throw std::runtime_error("Book not found"); }
bool BookRepository::adjustAvailability(int id, int difference) {
    const std::string statusCondition = difference < 0 ? " AND status='ACTIVE'" : "";
    db_.execute("UPDATE books SET available_quantity=available_quantity+?,updated_at=CURRENT_TIMESTAMP WHERE id=?" + statusCondition + " AND available_quantity+? BETWEEN 0 AND total_quantity", {std::to_string(difference),std::to_string(id),std::to_string(difference)});
    return db_.changes() == 1;
}

std::vector<Row> StudentRepository::list(const std::string& search, bool includeInactive) const {
    std::string sql="SELECT id,student_code,name,email,phone,department,course,study_year,registration_date,status FROM students WHERE 1=1"; std::vector<std::string> values;
    if(!includeInactive) sql += " AND status='ACTIVE'";
    if(!search.empty()) { sql += " AND (student_code LIKE ? OR name LIKE ? OR email LIKE ?)"; auto like="%"+search+"%"; values.insert(values.end(),{like,like,like}); }
    return db_.query(sql+" ORDER BY name",values);
}
std::optional<Row> StudentRepository::find(int id) const { return db_.one("SELECT id,student_code,name,email,phone,department,course,study_year,registration_date,status FROM students WHERE id=?", {std::to_string(id)}); }
int StudentRepository::create(const StudentInput& input, const std::string& passwordHash) {
    db_.execute("INSERT INTO students(student_code,name,email,password_hash,phone,department,course,study_year) VALUES(?,?,?,?,?,?,?,?)", {input.studentCode,input.name,input.email,passwordHash,input.phone,input.department,input.course,std::to_string(input.studyYear)}); return static_cast<int>(db_.lastInsertId());
}
void StudentRepository::update(int id, const StudentInput& input) { db_.execute("UPDATE students SET student_code=?,name=?,email=?,phone=?,department=?,course=?,study_year=? WHERE id=?", {input.studentCode,input.name,input.email,input.phone,input.department,input.course,std::to_string(input.studyYear),std::to_string(id)}); if(!db_.changes()) throw std::runtime_error("Student not found"); }
void StudentRepository::setStatus(int id, const std::string& status) { db_.execute("UPDATE students SET status=? WHERE id=?", {status,std::to_string(id)}); if(!db_.changes()) throw std::runtime_error("Student not found"); }

bool RequestRepository::hasActiveRequest(int studentId, int bookId) const { return db_.one("SELECT id FROM book_requests WHERE student_id=? AND book_id=? AND status='PENDING'", {std::to_string(studentId),std::to_string(bookId)}).has_value(); }
int RequestRepository::create(int studentId, int bookId) { db_.execute("INSERT INTO book_requests(student_id,book_id) VALUES(?,?)", {std::to_string(studentId),std::to_string(bookId)}); return static_cast<int>(db_.lastInsertId()); }
std::optional<Row> RequestRepository::find(int id) const { return db_.one("SELECT * FROM book_requests WHERE id=?", {std::to_string(id)}); }
std::vector<Row> RequestRepository::listAdmin(const std::string& status) const { std::string sql="SELECT r.*,s.name student_name,s.student_code,b.title book_title FROM book_requests r JOIN students s ON s.id=r.student_id JOIN books b ON b.id=r.book_id"; return status.empty() ? db_.query(sql+" ORDER BY r.request_date DESC") : db_.query(sql+" WHERE r.status=? ORDER BY r.request_date DESC",{status}); }
std::vector<Row> RequestRepository::listStudent(int studentId) const { return db_.query("SELECT r.*,b.title book_title,b.author FROM book_requests r JOIN books b ON b.id=r.book_id WHERE r.student_id=? ORDER BY r.request_date DESC",{std::to_string(studentId)}); }
void RequestRepository::approve(int id, int adminId) { db_.execute("UPDATE book_requests SET status='APPROVED',processed_date=CURRENT_TIMESTAMP,processed_by=? WHERE id=? AND status='PENDING'",{std::to_string(adminId),std::to_string(id)}); if(!db_.changes()) throw std::runtime_error("Request is no longer pending"); }
void RequestRepository::reject(int id, int adminId, const std::string& reason) { db_.execute("UPDATE book_requests SET status='REJECTED',processed_date=CURRENT_TIMESTAMP,processed_by=?,rejection_reason=? WHERE id=? AND status='PENDING'",{std::to_string(adminId),reason,std::to_string(id)}); if(!db_.changes()) throw std::runtime_error("Request is no longer pending"); }

bool IssueRepository::hasActiveIssue(int studentId, int bookId) const { return db_.one("SELECT id FROM issue_records WHERE student_id=? AND book_id=? AND status='ISSUED'", {std::to_string(studentId),std::to_string(bookId)}).has_value(); }
int IssueRepository::create(int studentId, int bookId, int adminId, const std::string& issueDate, const std::string& dueDate, const std::string& requestId) {
    if (requestId.empty()) {
        db_.execute("INSERT INTO issue_records(student_id,book_id,issued_by,issue_date,due_date) VALUES(?,?,?,?,?)", {std::to_string(studentId),std::to_string(bookId),std::to_string(adminId),issueDate,dueDate});
    } else {
        db_.execute("INSERT INTO issue_records(student_id,book_id,issued_by,request_id,issue_date,due_date) VALUES(?,?,?,?,?,?)", {std::to_string(studentId),std::to_string(bookId),std::to_string(adminId),requestId,issueDate,dueDate});
    }
    return static_cast<int>(db_.lastInsertId());
}
std::optional<Row> IssueRepository::find(int id) const { return db_.one("SELECT i.*,s.name student_name,b.title book_title FROM issue_records i JOIN students s ON s.id=i.student_id JOIN books b ON b.id=i.book_id WHERE i.id=?",{std::to_string(id)}); }
std::vector<Row> IssueRepository::listAdmin(const std::string& status) const { std::string sql="SELECT i.*,s.name student_name,s.student_code,b.title book_title FROM issue_records i JOIN students s ON s.id=i.student_id JOIN books b ON b.id=i.book_id"; return status.empty()?db_.query(sql+" ORDER BY i.issue_date DESC"):db_.query(sql+" WHERE i.status=? ORDER BY i.issue_date DESC",{status}); }
std::vector<Row> IssueRepository::listStudent(int studentId, bool history) const { std::string sql="SELECT i.*,b.title book_title,b.author FROM issue_records i JOIN books b ON b.id=i.book_id WHERE i.student_id=?"; if(!history) sql+=" AND i.status='ISSUED'"; return db_.query(sql+" ORDER BY i.issue_date DESC",{std::to_string(studentId)}); }
void IssueRepository::returnBook(int id, const std::string& returnDate, double fine) { db_.execute("UPDATE issue_records SET status='RETURNED',return_date=?,fine=? WHERE id=? AND status='ISSUED'", {returnDate,std::to_string(fine),std::to_string(id)}); if(!db_.changes()) throw std::runtime_error("Issue is already returned or does not exist"); }

void ActivityRepository::create(const std::string& actorType, int actorId, const std::string& event, const std::string& message) { db_.execute("INSERT INTO activity_logs(actor_type,actor_id,event_type,message) VALUES(?,?,?,?)", {actorType,std::to_string(actorId),event,message}); }
std::vector<Row> ActivityRepository::recent(int limit) const { return db_.query("SELECT * FROM activity_logs ORDER BY created_at DESC LIMIT ?", {std::to_string(limit)}); }
Row ReportRepository::summary() const { auto result=db_.one("SELECT (SELECT COUNT(*) FROM books WHERE status='ACTIVE') total_books,(SELECT COUNT(*) FROM books WHERE status='ACTIVE' AND available_quantity>0) available_books,(SELECT COUNT(*) FROM issue_records WHERE status='ISSUED') issued_books,(SELECT COUNT(*) FROM students WHERE status='ACTIVE') total_students,(SELECT COUNT(*) FROM book_requests WHERE status='PENDING') pending_requests,(SELECT COUNT(*) FROM issue_records WHERE status='ISSUED' AND due_date < date('now')) overdue_books"); return result.value_or(Row{}); }
} // namespace library
