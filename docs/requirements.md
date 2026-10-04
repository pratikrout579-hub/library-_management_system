# Requirements

## Functional

- Admins authenticate and manage books, student accounts, requests, issues, returns, reports and transactions.
- Students authenticate, search/filter active books, submit one pending request per title, view active loans, history and profile.
- Approval creates an issue and decrements stock inside a database transaction; return changes issue status, adds stock and computes `overdue days × 5`.
- Every important action creates a database activity row and an event sent to the Linux character device driver through /dev/library_device.

## Non-functional

- The UI supports current desktop, tablet and mobile browsers.
- SQLite is accessed only in the C++ repository layer through prepared statements.
- Server routes enforce roles and student ownership; client route protection is a convenience, not the security boundary.
