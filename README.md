# Smart Library Management System

A role-based web application for managing a college library. Administrators manage books, students, requests, issues, returns and reports; students browse the catalogue, request books and follow their borrowing history.

## Technology stack

- Frontend (website only): HTML5, CSS3 and vanilla JavaScript modules
- Core application / backend: C++20 with the Crow REST framework
- Database: SQLite
- Linux kernel driver: C (character device driver built as a kernel module, `/dev/library_device`)
- Build: CMake for the application, Kbuild `make` for the kernel module
- Platform: Linux. The project is intended to be built and demonstrated on Linux.

## Architecture

The project includes a Linux character device driver implemented as a kernel module. The driver creates `/dev/library_device` and communicates with the C/C++ application through Linux device-file operations. The driver is an integral component of the complete Linux architecture and is used when demonstrating the full system.

```text
HTML / CSS / JavaScript
        ↓
C++ Backend
        ↓
/dev/library_device
        ↓
Linux Character Device Driver
        ↓
Linux Kernel
```

- HTML/CSS/JavaScript are used only for the website frontend.
- The core application/backend is C/C++ (Crow REST API, services, repositories).
- The Linux kernel driver (`driver/library_device.c`) is written in C.
- SQLite is used for database storage.
- Inside the backend, the layers are controllers → services → repositories → SQLite; library events (`BOOK_ISSUED`, `BOOK_RETURNED`, `BOOK_ADDED`, `BOOK_REMOVED`, ...) are additionally written by the `ActivityLogger` to `/dev/library_device`.

## Features

- Secure role-based login with SHA-256 password hashes (demo-only; see limitations)
- Admin book and student management, request approvals/rejections, direct issues, returns, transactions and reports
- Student catalogue search/filtering, request workflow, active loans and history
- SQLite foreign keys, backend validation, transactional issue/return workflows and activity logs
- Responsive sidebar UI, modals, loading/empty/error states and toast notifications

## Quick start on Linux

```bash
sudo apt update
sudo apt install build-essential cmake libsqlite3-dev libssl-dev
cmake -S . -B build
cmake --build build
./build/backend/smart-library --init-db --db database/library.db
./build/backend/smart-library --db database/library.db --port 18080
```

Open `http://localhost:18080`. The Crow dependency is fetched by CMake on the first configure; an internet connection is needed then.

## Linux device driver

`driver/library_device.c` is a real Linux kernel module. It uses `module_init`/`module_exit`, registers a character device, implements `struct file_operations` (`open`, `read`, `write`, `release`, `unlocked_ioctl`), exchanges data with user space through `copy_from_user`/`copy_to_user`, creates the `/dev/library_device` node automatically, and logs every event with `printk`. The C++ backend opens `/dev/library_device` and writes one event per library action; the driver buffers the events and the kernel log shows them.

### Demonstrating the driver

Prerequisites (Debian/Ubuntu): `sudo apt install build-essential cmake libsqlite3-dev libssl-dev linux-headers-$(uname -r)`

**1. Build and load the driver**

```bash
cd driver
make                                  # produces library_device.ko
sudo insmod library_device.ko         # load the module
lsmod | grep library_device           # module is listed
ls -l /dev/library_device             # device node exists (character device)
sudo dmesg | tail                     # "library_device: loaded, /dev/library_device ..."
cd ..
```

**2. Build and run the application** (from the project root, the driver stays loaded)

```bash
cmake -S . -B build
cmake --build build
./build/backend/smart-library --db database/library.db --port 18080
```

Open `http://localhost:18080`, log in as the admin (see demo credentials), and add a book, issue a book or return a book.

**3. Check that the application communicated with the driver** (second terminal)

```bash
sudo dmesg | tail                     # kernel log: "library_device: event #N: BOOK_ISSUED|<title>"
cat /dev/library_device               # reads (and consumes) the buffered events
```

**4. Test the device file manually**

```bash
echo "BOOK_ISSUED|manual test" > /dev/library_device
cat /dev/library_device               # prints: BOOK_ISSUED|manual test
sudo dmesg | tail
```

**5. Unload the driver**

```bash
sudo rmmod library_device
sudo dmesg | tail                     # "library_device: unloaded ..."
cd driver && make clean && cd ..
```

Load the driver before issuing library events in the UI; events are delivered to the kernel whenever `/dev/library_device` exists.

## How this project satisfies the requirements

1. **Programming Language:** C/C++ for the core application and C for the Linux kernel module; HTML/CSS/JavaScript only for the frontend.
2. **Operating System:** Linux.
3. **Linux Device Driver:** A real Linux character-device kernel module that creates `/dev/library_device`.
4. **Architecture:** Layered software architecture (frontend → controllers → services → repositories → SQLite) plus Linux kernel/device-driver architecture (backend → `/dev/library_device` → character device driver → Linux kernel).

## Demo credentials

| Role | Email | Password |
|---|---|---|
| Admin | admin@smartlibrary.edu | admin123 |
| Student | aarav.sharma@college.edu | student123 |

## Project map

```text
frontend/       Static HTML, CSS and JavaScript modules
backend/        Crow controllers, services, repositories and models
database/       SQLite schema and realistic development seed data
driver/         Linux character device driver (kernel module, C): library_device.c, library_device.h, Makefile
docs/           Architecture, API, testing, UML and deployment notes
```

Read [docs/architecture.md](docs/architecture.md), [docs/api-documentation.md](docs/api-documentation.md), and [docs/testing.md](docs/testing.md) before presenting or extending the project.

## Limitations

- Sessions are in-memory and are cleared whenever the server restarts.
- Password hashing is SHA-256 for an understandable capstone demonstration; production software should use Argon2id or bcrypt with per-user salts.
- The kernel module must be built against the headers of the running kernel and loaded with root privileges (`insmod`). The backend does not crash if the device file is absent (for example when only the website is being browsed on a machine where the module is not loaded), but the complete system is demonstrated with the driver loaded.

## Future improvements

Persisted sessions, Argon2id passwords, pagination, CSV/PDF report exports, email reminders and barcode scanning are sensible next steps.
