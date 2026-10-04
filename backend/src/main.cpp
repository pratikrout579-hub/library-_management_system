#include <crow.h>

#include <filesystem>
#include <iostream>
#include <string>
#include "controllers/Routes.h"
#include "repositories/Database.h"
#include "repositories/LibraryRepositories.h"
#include "services/AuthService.h"
#include "services/LibraryService.h"
#include "services/ReportService.h"
#include "utils/ActivityLogger.h"

int main(int argc, char* argv[]) {
    std::string databasePath = std::string(PROJECT_ROOT) + "/database/library.db";
    int port = 18080;
    bool initialize = false;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--db" && index + 1 < argc) databasePath = argv[++index];
        else if (argument == "--port" && index + 1 < argc) port = std::stoi(argv[++index]);
        else if (argument == "--init-db") initialize = true;
        else if (argument == "--help") { std::cout << "Usage: smart-library [--init-db] [--db path] [--port number]\n"; return 0; }
    }
    try {
        std::filesystem::create_directories(std::filesystem::path(databasePath).parent_path());
        library::Database database(databasePath);
        if (initialize) {
            database.executeScript(std::string(PROJECT_ROOT) + "/database/schema.sql");
            database.executeScript(std::string(PROJECT_ROOT) + "/database/seed.sql");
            std::cout << "Database initialized: " << databasePath << '\n';
            return 0;
        }

        library::ActivityLogger logger(std::string(PROJECT_ROOT) + "/logs/activity.log");
        library::AuthRepository authRepository(database);
        library::BookRepository books(database);
        library::StudentRepository students(database);
        library::RequestRepository requests(database);
        library::IssueRepository issues(database);
        library::ActivityRepository activities(database);
        library::ReportRepository reportRepository(database);
        library::AuthService auth(authRepository, activities, logger);
        library::LibraryService libraryService(database, books, students, requests, issues, activities, logger);
        library::ReportService reportService(reportRepository, issues, requests, activities);

        crow::SimpleApp app;
        CROW_ROUTE(app, "/")([] { crow::response response(302); response.set_header("Location", "/index.html"); return response; });
        library::registerRoutes(app, auth, libraryService, books, students, requests, issues, reportService);
        std::cout << "Smart Library running at http://localhost:" << port << "\n";
        app.port(static_cast<uint16_t>(port)).run();
    } catch (const std::exception& error) {
        std::cerr << "Startup error: " << error.what() << '\n';
        return 1;
    }
}
