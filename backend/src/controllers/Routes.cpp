#include "controllers/Routes.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace library {
namespace {
using Json = crow::json::wvalue;

crow::response send(int status, Json body) {
    crow::response response{status, std::move(body)};
    response.set_header("Content-Type", "application/json");
    response.set_header("Access-Control-Allow-Origin", "*");
    response.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    return response;
}
crow::response fail(int status, const std::string& message, const std::string& detail = "") {
    Json body; body["success"] = false; body["message"] = message; body["error"] = detail.empty() ? message : detail;
    return send(status, std::move(body));
}
crow::response ok(const std::string& message, Json data = Json{}) {
    Json body; body["success"] = true; body["message"] = message; body["data"] = std::move(data);
    return send(200, std::move(body));
}
Json asJson(const Row& row) { Json value; for (const auto& [key, text] : row) value[key] = text; return value; }
Json::list rowList(const std::vector<Row>& rows) { Json::list values; for (const auto& row : rows) values.emplace_back(asJson(row)); return values; }
Json asList(const std::vector<Row>& rows) { Json data; data["items"] = rowList(rows); return data; }
std::string text(const crow::json::rvalue& body, const char* key) {
    return body.has(key) ? std::string(body[key].s()) : std::string{};
}
int number(const crow::json::rvalue& body, const char* key) { return body.has(key) ? static_cast<int>(body[key].i()) : 0; }
std::string parameter(const crow::request& request, const char* key) { const auto* value = request.url_params.get(key); return value ? std::string(value) : ""; }
std::string token(const crow::request& request) {
    const auto authorization=request.get_header_value("Authorization"); const std::string prefix="Bearer ";
    return authorization.rfind(prefix,0)==0 ? authorization.substr(prefix.size()) : "";
}
std::string currentDate() {
    const auto now=std::time(nullptr); std::tm local{};
    localtime_r(&now,&local);
    std::ostringstream value; value<<std::put_time(&local,"%F"); return value.str();
}
std::optional<SessionUser> authorized(const crow::request& request, AuthService& auth, const std::string& role) {
    auto user=auth.userFromToken(token(request)); if(!user || (!role.empty() && user->role!=role)) return std::nullopt; return user;
}
BookInput bookInput(const crow::json::rvalue& body) { return {text(body,"title"),text(body,"author"),text(body,"category"),text(body,"isbn"),text(body,"publisher"),number(body,"publication_year"),number(body,"total_quantity")}; }
StudentInput studentInput(const crow::json::rvalue& body) { return {text(body,"student_code"),text(body,"name"),text(body,"email"),text(body,"password"),text(body,"phone"),text(body,"department"),text(body,"course"),number(body,"study_year")}; }
template <typename Action> crow::response guarded(Action action) { try { return action(); } catch(const std::exception& error) { return fail(400,error.what()); } }
} // namespace

void registerRoutes(crow::SimpleApp& app, AuthService& auth, LibraryService& library, BookRepository& books,
                    StudentRepository& students, RequestRepository& requests, IssueRepository& issues, ReportService& reports) {
    CROW_ROUTE(app, "/api/<path>").methods(crow::HTTPMethod::OPTIONS)([](std::string) { return send(204, Json{}); });

    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::POST)([&auth](const crow::request& request) {
        return guarded([&] { auto body=crow::json::load(request.body); if(!body) return fail(400,"Invalid JSON request"); auto session=auth.login(text(body,"email"),text(body,"password")); auto user=auth.userFromToken(session).value(); Json data=asJson({{"token",session},{"id",std::to_string(user.id)},{"role",user.role},{"name",user.name},{"email",user.email}}); return ok("Login successful",std::move(data)); });
    });
    CROW_ROUTE(app, "/api/auth/logout").methods(crow::HTTPMethod::POST)([&auth](const crow::request& request) { auth.logout(token(request)); return ok("Logged out"); });
    CROW_ROUTE(app, "/api/auth/me").methods(crow::HTTPMethod::GET)([&auth](const crow::request& request) { auto user=authorized(request,auth,""); if(!user) return fail(401,"Authentication required"); return ok("Current user",asJson({{"id",std::to_string(user->id)},{"role",user->role},{"name",user->name},{"email",user->email}})); });

    CROW_ROUTE(app, "/api/books").methods(crow::HTTPMethod::GET)([&books](const crow::request& request) { return guarded([&] { return ok("Books loaded",asList(books.list(parameter(request,"search"),parameter(request,"category"),parameter(request,"availability")))); }); });
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::GET)([&books](int id) { auto book=books.find(id); return book?ok("Book loaded",asJson(*book)):fail(404,"Book not found"); });

    CROW_ROUTE(app, "/api/admin/books").methods(crow::HTTPMethod::GET)([&](const crow::request& request) {
        if (!authorized(request, auth, "ADMIN")) return fail(403, "Admin authorization required");
        return ok("Books loaded", asList(books.list(parameter(request, "search"), parameter(request, "category"), parameter(request, "availability"), true)));
    });
    CROW_ROUTE(app, "/api/admin/books").methods(crow::HTTPMethod::POST)([&](const crow::request& request) { auto user=authorized(request,auth,"ADMIN"); if(!user) return fail(403,"Admin authorization required"); return guarded([&]{auto body=crow::json::load(request.body); if(!body)return fail(400,"Invalid JSON request"); auto id=library.addBook(bookInput(body),user->id); return ok("Book added",asJson({{"id",std::to_string(id)}}));}); });
    CROW_ROUTE(app, "/api/admin/books/<int>").methods(crow::HTTPMethod::PUT)([&](const crow::request& request,int id) { auto user=authorized(request,auth,"ADMIN"); if(!user)return fail(403,"Admin authorization required"); return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");library.updateBook(id,bookInput(body),user->id);return ok("Book updated");}); });
    CROW_ROUTE(app, "/api/admin/books/<int>").methods(crow::HTTPMethod::DELETE)([&](const crow::request& request,int id) { auto user=authorized(request,auth,"ADMIN"); if(!user)return fail(403,"Admin authorization required"); return guarded([&]{library.deactivateBook(id,user->id);return ok("Book deactivated");}); });

    CROW_ROUTE(app, "/api/admin/students").methods(crow::HTTPMethod::GET)([&](const crow::request& request) { if(!authorized(request,auth,"ADMIN"))return fail(403,"Admin authorization required"); return ok("Students loaded",asList(students.list(parameter(request,"search"),true))); });
    CROW_ROUTE(app, "/api/admin/students/<int>").methods(crow::HTTPMethod::GET)([&](const crow::request& request,int id) { if(!authorized(request,auth,"ADMIN"))return fail(403,"Admin authorization required");auto student=students.find(id);return student?ok("Student loaded",asJson(*student)):fail(404,"Student not found"); });
    CROW_ROUTE(app, "/api/admin/students").methods(crow::HTTPMethod::POST)([&](const crow::request& request) {auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");auto id=library.addStudent(studentInput(body),user->id);return ok("Student added",asJson({{"id",std::to_string(id)}}));});});
    CROW_ROUTE(app, "/api/admin/students/<int>").methods(crow::HTTPMethod::PUT)([&](const crow::request& request,int id) {auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");library.updateStudent(id,studentInput(body),user->id);return ok("Student updated");});});
    CROW_ROUTE(app, "/api/admin/students/<int>/status").methods(crow::HTTPMethod::PATCH)([&](const crow::request& request,int id) {auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");library.setStudentStatus(id,text(body,"status"),user->id);return ok("Student status updated");});});

    CROW_ROUTE(app, "/api/student/requests").methods(crow::HTTPMethod::POST)([&](const crow::request& request) {auto user=authorized(request,auth,"STUDENT");if(!user)return fail(403,"Student authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");auto id=library.requestBook(user->id,number(body,"book_id"));return ok("Book request submitted",asJson({{"id",std::to_string(id)}}));});});
    CROW_ROUTE(app, "/api/student/requests").methods(crow::HTTPMethod::GET)([&](const crow::request& request){auto user=authorized(request,auth,"STUDENT");if(!user)return fail(403,"Student authorization required");return ok("Requests loaded",asList(requests.listStudent(user->id)));});
    CROW_ROUTE(app, "/api/admin/requests").methods(crow::HTTPMethod::GET)([&](const crow::request& request){if(!authorized(request,auth,"ADMIN"))return fail(403,"Admin authorization required");return ok("Requests loaded",asList(requests.listAdmin(parameter(request,"status"))));});
    CROW_ROUTE(app, "/api/admin/requests/<int>/approve").methods(crow::HTTPMethod::PATCH)([&](const crow::request& request,int id){auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");library.approveRequest(id,user->id,text(body,"due_date"));return ok("Request approved and book issued");});});
    CROW_ROUTE(app, "/api/admin/requests/<int>/reject").methods(crow::HTTPMethod::PATCH)([&](const crow::request& request,int id){auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");library.rejectRequest(id,user->id,text(body,"reason"));return ok("Request rejected");});});

    CROW_ROUTE(app, "/api/admin/issues").methods(crow::HTTPMethod::GET)([&](const crow::request& request){if(!authorized(request,auth,"ADMIN"))return fail(403,"Admin authorization required");return ok("Issues loaded",asList(issues.listAdmin(parameter(request,"status"))));});
    CROW_ROUTE(app, "/api/admin/issues").methods(crow::HTTPMethod::POST)([&](const crow::request& request){auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");auto id=library.directIssue(number(body,"student_id"),number(body,"book_id"),user->id,text(body,"issue_date"),text(body,"due_date"));return ok("Book issued",asJson({{"id",std::to_string(id)}}));});});
    CROW_ROUTE(app, "/api/student/issues").methods(crow::HTTPMethod::GET)([&](const crow::request& request){auto user=authorized(request,auth,"STUDENT");if(!user)return fail(403,"Student authorization required");return ok("Active issues loaded",asList(issues.listStudent(user->id,false)));});
    CROW_ROUTE(app, "/api/student/profile").methods(crow::HTTPMethod::GET)([&](const crow::request& request){auto user=authorized(request,auth,"STUDENT");if(!user)return fail(403,"Student authorization required");auto student=students.find(user->id);return student?ok("Profile loaded",asJson(*student)):fail(404,"Student profile not found");});
    CROW_ROUTE(app, "/api/admin/issues/<int>/return").methods(crow::HTTPMethod::PATCH)([&](const crow::request& request,int id){auto user=authorized(request,auth,"ADMIN");if(!user)return fail(403,"Admin authorization required");return guarded([&]{auto body=crow::json::load(request.body);if(!body)return fail(400,"Invalid JSON request");auto fine=library.returnIssue(id,user->id,text(body,"return_date"));return ok("Book returned",asJson({{"fine",std::to_string(fine)}}));});});

    CROW_ROUTE(app, "/api/admin/transactions").methods(crow::HTTPMethod::GET)([&](const crow::request& request){if(!authorized(request,auth,"ADMIN"))return fail(403,"Admin authorization required");return ok("Transactions loaded",asList(issues.listAdmin(parameter(request,"status"))));});
    CROW_ROUTE(app, "/api/student/history").methods(crow::HTTPMethod::GET)([&](const crow::request& request){auto user=authorized(request,auth,"STUDENT");if(!user)return fail(403,"Student authorization required");return ok("History loaded",asList(issues.listStudent(user->id,true)));});

    CROW_ROUTE(app, "/api/admin/reports").methods(crow::HTTPMethod::GET)([&](const crow::request& request){if(!authorized(request,auth,"ADMIN"))return fail(403,"Admin authorization required");Json data;data["summary"]=asJson(reports.summary());data["transactions"]=rowList(reports.recentTransactions());data["requests"]=rowList(reports.recentRequests());data["activity"]=rowList(reports.recentActivity());return ok("Report loaded",std::move(data));});
    CROW_ROUTE(app, "/api/student/dashboard").methods(crow::HTTPMethod::GET)([&](const crow::request& request){auto user=authorized(request,auth,"STUDENT");if(!user)return fail(403,"Student authorization required");auto active=issues.listStudent(user->id,false);auto requestRows=requests.listStudent(user->id);int pending=0,approved=0,overdue=0;for(const auto& row:requestRows){if(row.at("status")=="PENDING")++pending;if(row.at("status")=="APPROVED")++approved;}const auto today=currentDate();for(const auto& row:active){if(row.at("due_date")<today)++overdue;}Json data;data["currently_borrowed"]=std::to_string(active.size());data["pending_requests"]=std::to_string(pending);data["approved_requests"]=std::to_string(approved);data["overdue_books"]=std::to_string(overdue);data["recent_requests"]=rowList(requestRows);return ok("Student dashboard loaded",std::move(data));});
}
} // namespace library
