PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS admins (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  email TEXT NOT NULL UNIQUE,
  password_hash TEXT NOT NULL,
  status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE','INACTIVE')),
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS students (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  student_code TEXT NOT NULL UNIQUE,
  name TEXT NOT NULL,
  email TEXT NOT NULL UNIQUE,
  password_hash TEXT NOT NULL,
  phone TEXT,
  department TEXT NOT NULL,
  course TEXT NOT NULL,
  study_year INTEGER NOT NULL CHECK(study_year BETWEEN 1 AND 8),
  registration_date TEXT NOT NULL DEFAULT CURRENT_DATE,
  status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE','INACTIVE'))
);

CREATE TABLE IF NOT EXISTS books (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  title TEXT NOT NULL,
  author TEXT NOT NULL,
  category TEXT NOT NULL,
  isbn TEXT NOT NULL UNIQUE,
  publisher TEXT,
  publication_year INTEGER CHECK(publication_year BETWEEN 1000 AND 2100),
  total_quantity INTEGER NOT NULL CHECK(total_quantity > 0),
  available_quantity INTEGER NOT NULL CHECK(available_quantity >= 0 AND available_quantity <= total_quantity),
  status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE','INACTIVE')),
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS book_requests (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  student_id INTEGER NOT NULL REFERENCES students(id),
  book_id INTEGER NOT NULL REFERENCES books(id),
  request_date TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
  status TEXT NOT NULL DEFAULT 'PENDING' CHECK(status IN ('PENDING','APPROVED','REJECTED','CANCELLED')),
  processed_date TEXT,
  processed_by INTEGER REFERENCES admins(id),
  rejection_reason TEXT
);

CREATE TABLE IF NOT EXISTS issue_records (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  student_id INTEGER NOT NULL REFERENCES students(id),
  book_id INTEGER NOT NULL REFERENCES books(id),
  issued_by INTEGER REFERENCES admins(id),
  request_id INTEGER UNIQUE REFERENCES book_requests(id),
  issue_date TEXT NOT NULL,
  due_date TEXT NOT NULL,
  return_date TEXT,
  fine REAL NOT NULL DEFAULT 0 CHECK(fine >= 0),
  status TEXT NOT NULL DEFAULT 'ISSUED' CHECK(status IN ('ISSUED','RETURNED')),
  CHECK(due_date >= issue_date)
);

CREATE TABLE IF NOT EXISTS activity_logs (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  actor_type TEXT NOT NULL CHECK(actor_type IN ('ADMIN','STUDENT','SYSTEM')),
  actor_id INTEGER,
  event_type TEXT NOT NULL,
  message TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_books_search ON books(title, author, category, status);
CREATE INDEX IF NOT EXISTS idx_requests_student ON book_requests(student_id, status);
CREATE INDEX IF NOT EXISTS idx_issues_student ON issue_records(student_id, status);
CREATE UNIQUE INDEX IF NOT EXISTS idx_one_pending_request ON book_requests(student_id, book_id) WHERE status = 'PENDING';
CREATE UNIQUE INDEX IF NOT EXISTS idx_one_active_issue ON issue_records(student_id, book_id) WHERE status = 'ISSUED';
