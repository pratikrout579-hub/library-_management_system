# Linux/C++ integration

The web service is designed for Linux and built with CMake. Install `build-essential`, `cmake`, `libsqlite3-dev`, and `libssl-dev`; CMake fetches Crow when configuring.

```bash
cmake -S . -B build
cmake --build build
./build/backend/smart-library --init-db --db database/library.db
./build/backend/smart-library --db database/library.db --port 18080
journalctl -f             # if managed by systemd
tail -f logs/activity.log
ss -ltnp | grep 18080
```

The Linux character device driver (`driver/library_device.c`, C kernel module) is an integral part of the architecture. The C++ backend (`ActivityLogger`) writes each circulation event to `/dev/library_device`, and the driver buffers it and logs it to the kernel log. See the top-level README, section "Demonstrating the driver", for the exact build, load, test and unload commands. The backend tolerates a missing device file so that a circulation operation is never aborted by a logging problem, but the complete system is demonstrated with the module loaded.
