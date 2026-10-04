# Deployment notes

Run the process under an unprivileged `smartlibrary` Linux account. Keep `database/library.db` writable only by that account, place a reverse proxy with TLS in front of port 18080, and back up the database before schema changes.

For a simple systemd service, use `WorkingDirectory` at the repository root and `ExecStart=/path/to/smart-library --db /var/lib/smart-library/library.db --port 18080`. Verify with `systemctl status`, `journalctl -u smart-library`, and a request to `/api/books`.
