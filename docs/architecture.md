# Architecture

```mermaid
flowchart LR
  UI[HTML/CSS/Vanilla JS] -->|Fetch JSON| Routes[Crow controllers]
  Routes --> Services[Business services]
  Services --> Repositories[SQLite repositories]
  Repositories --> DB[(SQLite)]
  Services --> Logs[Activity repository]
  Services -->|write events| Dev[/dev/library_device]
  Dev --> Driver[Linux character device driver]
  Driver --> Kernel[Linux kernel log and buffer]
```

The project includes a Linux character device driver implemented as a kernel module (`driver/library_device.c`, written in C). The driver creates `/dev/library_device` and communicates with the C/C++ application through Linux device-file operations (`open`, `write`, `read`, `ioctl`, `close`). The driver is an integral component of the complete Linux architecture and is used when demonstrating the full system. HTML/CSS/JavaScript are used only for the website frontend, the core application is C++, and SQLite stores the data.

Controllers parse HTTP, authorize a request and select an HTTP response. Services own workflow rules and transactions. Repositories contain SQL and bind all external values as parameters. This prevents the frontend from reading SQLite directly and makes business rules testable without page code.

Sessions are held in memory as random bearer tokens for capstone scope. On a production deployment use secure cookies or a persisted, expiring token store behind HTTPS.
