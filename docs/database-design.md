# Database design

| Table | Purpose | Important relationships |
|---|---|---|
| `admins` | Library staff credentials | Processes requests/issues |
| `students` | Student accounts/profile | Owns requests and issue records |
| `books` | Catalogue and copy counts | Referenced by requests and issues |
| `book_requests` | PENDING/APPROVED/REJECTED workflow | Student + book + processing admin |
| `issue_records` | Borrow/return transaction | Student + book + optional request |
| `activity_logs` | Audit-style activity trail | Actor type/id + event |

Quantity constraints are enforced in SQL: `total_quantity > 0`, `available_quantity >= 0`, and available may not exceed total. Foreign keys are enabled whenever the database opens.

The schema avoids deleting historical issues. A book is deactivated by status, preserving references and reporting accuracy.
