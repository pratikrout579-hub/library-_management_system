#pragma once
#include <crow.h>
#include "services/AuthService.h"
#include "services/LibraryService.h"
#include "services/ReportService.h"

namespace library {
void registerRoutes(crow::SimpleApp& app, AuthService& auth, LibraryService& library, BookRepository& books,
                    StudentRepository& students, RequestRepository& requests, IssueRepository& issues,
                    ReportService& reports);
} // namespace library
