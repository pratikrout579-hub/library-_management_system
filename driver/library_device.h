/* Shared kernel/user-space ioctl definitions for /dev/library_device. */
#ifndef LIBRARY_DEVICE_H
#define LIBRARY_DEVICE_H

#include <linux/ioctl.h>

#define LIBRARY_IOC_MAGIC       'L'
#define LIBRARY_IOC_GET_COUNT   _IOR(LIBRARY_IOC_MAGIC, 1, unsigned long)
#define LIBRARY_IOC_CLEAR       _IO(LIBRARY_IOC_MAGIC, 2)
#define LIBRARY_IOC_GET_PENDING _IOR(LIBRARY_IOC_MAGIC, 3, unsigned long)

#endif
