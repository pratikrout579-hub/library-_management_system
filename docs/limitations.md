# Known limitations

- In-memory sessions end when the backend restarts and do not expire automatically.
- SHA-256 is demonstrative password hashing, not production-grade password storage. Use Argon2id/bcrypt plus unique salts in a real deployment.
- List views do not yet paginate or export data.
- The kernel module must be built against the headers of the running kernel and loaded with root privileges. The backend tolerates a missing device file, but the full system is demonstrated with the driver loaded.
- Email reminders, password reset, barcode scanning and payments are outside this capstone scope.
