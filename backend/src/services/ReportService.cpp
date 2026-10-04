#include "services/ReportService.h"
namespace library {
Row ReportService::summary() const { return reports_.summary(); }
std::vector<Row> ReportService::recentTransactions() const { return issues_.listAdmin(""); }
std::vector<Row> ReportService::recentRequests() const { return requests_.listAdmin(""); }
std::vector<Row> ReportService::recentActivity() const { return activity_.recent(); }
} // namespace library
