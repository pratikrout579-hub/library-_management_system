# UML diagrams

## Use case diagram

```mermaid
flowchart LR
 Student((Student)) --> Browse[Browse/search books]
 Student --> Request[Request book]
 Student --> Mine[View loans and history]
 Admin((Admin)) --> ManageBooks[Manage books]
 Admin --> ManageStudents[Manage students]
 Admin --> Decide[Approve/reject requests]
 Admin --> Circulate[Issue/return books]
 Admin --> Reports[View reports]
 Decide --> Circulate
```

## Class diagram

```mermaid
classDiagram
 class Book { +id +title +isbn +totalQuantity +availableQuantity +status }
 class Student { +id +studentCode +email +status }
 class BookRequest { +id +status +requestDate +processedDate }
 class IssueRecord { +id +issueDate +dueDate +returnDate +fine +status }
 class LibraryService { +requestBook() +approveRequest() +directIssue() +returnIssue() }
 class BookRepository
 class StudentRepository
 class RequestRepository
 class IssueRepository
 Student "1" --> "many" BookRequest
 Book "1" --> "many" BookRequest
 Student "1" --> "many" IssueRecord
 Book "1" --> "many" IssueRecord
 LibraryService --> BookRepository
 LibraryService --> StudentRepository
 LibraryService --> RequestRepository
 LibraryService --> IssueRepository
```

## Request approval sequence

```mermaid
sequenceDiagram
 participant S as Student UI
 participant A as Admin UI
 participant API as Crow API
 participant L as LibraryService
 participant DB as SQLite
 S->>API: POST request(bookId)
 API->>L: validate student/book/no duplicate
 L->>DB: INSERT PENDING request
 A->>API: PATCH approve(dueDate)
 API->>L: approve request
 L->>DB: BEGIN; decrement copy; insert issue; approve request; log; COMMIT
 API-->>A: success
```

## Return sequence

```mermaid
sequenceDiagram
 participant A as Admin
 participant API as Crow API
 participant L as LibraryService
 participant DB as SQLite
 A->>API: PATCH issue/return(returnDate)
 API->>L: validate active issue, calculate fine
 L->>DB: BEGIN; mark RETURNED; increment copy; log; COMMIT
 API-->>A: fine and success
```

## State diagram

```mermaid
stateDiagram-v2
 [*] --> PENDING
 PENDING --> APPROVED
 PENDING --> REJECTED
 PENDING --> CANCELLED
 APPROVED --> ISSUED
 ISSUED --> RETURNED
```

## System architecture diagram

```mermaid
flowchart TB
 Browser[Browser: vanilla JS] --> Crow[Crow HTTP controllers]
 Crow --> Services[Services and validation]
 Services --> Repositories[Prepared SQL repositories]
 Repositories --> SQLite[(SQLite)]
 Services --> DBLog[activity_logs]
 Services --> FileLog[Linux kernel driver /dev/library_device]
```
