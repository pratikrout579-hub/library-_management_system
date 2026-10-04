# Linux character device driver

`library_device.c` is a real Linux kernel module (C). It registers a character device and exposes `/dev/library_device`.

Concepts used: `module_init`/`module_exit`, `alloc_chrdev_region`, `cdev_add`, `struct file_operations` (`open`, `read`, `write`, `release`, `unlocked_ioctl`), `copy_from_user`/`copy_to_user`, `class_create`/`device_create` (automatic `/dev` node), mutex locking, and `printk` logging.

- `write()` receives an event such as `BOOK_ISSUED|Clean Code` from the C++ backend.
- `read()` returns the buffered events (consumed once read).
- `ioctl()` supports get-count, get-pending-bytes and clear (see `library_device.h`).

The driver is an integral component of the project's Linux architecture: HTML/CSS/JavaScript frontend → C++ backend → `/dev/library_device` → character device driver → Linux kernel.

Build, load, test and unload commands are in the top-level README (section "Demonstrating the driver"). From this directory: `make`, `sudo insmod library_device.ko`, `ls -l /dev/library_device`, `lsmod | grep library_device`, `sudo dmesg | tail`, `sudo rmmod library_device`.
