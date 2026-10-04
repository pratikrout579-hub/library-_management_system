#pragma once

#include "repositories/LibraryRepositories.h"

namespace library {
class ReportService {
public:
    ReportService(ReportRepository& reports, IssueRepository& issues, RequestRepository& requests, ActivityRepository& activity)
        : reports_(reports), issues_(issues), requests_(requests), activity_(activity) {}
    Row summary() const;
    std::vector<Row> recentTransactions() const;
    std::vector<Row> recentRequests() const;
    std::vector<Row> recentActivity() const;
private:
    ReportRepository& reports_; IssueRepository& issues_; RequestRepository& requests_; ActivityRepository& activity_;
};
} // namespace library
