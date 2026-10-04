// SPDX-License-Identifier: GPL-2.0
/*
 * library_device.c - Linux character device driver for the Smart Library
 * Management System.
 *
 * Creates /dev/library_device. The C++ backend writes one event per write()
 * (e.g. "BOOK_ISSUED|Clean Code"); the driver keeps a ring buffer of recent
 * events, counts them, logs them with printk() and returns them via read().
 *
 * ioctl commands (see driver/library_device.h):
 *   LIBRARY_IOC_GET_COUNT   - total events received since load
 *   LIBRARY_IOC_CLEAR       - empty the event buffer and reset the counter
 *   LIBRARY_IOC_GET_PENDING - bytes currently waiting in the buffer
 */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include "library_device.h"

#define DEVICE_NAME  "library_device"
#define CLASS_NAME   "library"
#define BUF_SIZE     4096
#define MAX_EVENT    256

static dev_t dev_number;
static struct cdev library_cdev;
static struct class *library_class;
static struct device *library_dev;

static DEFINE_MUTEX(library_lock);
static char *event_buf;          /* ring buffer of newline-separated events */
static size_t buf_head;          /* next read position  */
static size_t buf_len;           /* bytes stored        */
static unsigned long event_count;

static int library_open(struct inode *inode, struct file *filp)
{
	pr_info("library_device: opened by pid %d (%s)\n",
		current->pid, current->comm);
	return 0;
}

static int library_release(struct inode *inode, struct file *filp)
{
	pr_info("library_device: closed by pid %d\n", current->pid);
	return 0;
}

/* Copy buffered events to user space, consuming them. */
static ssize_t library_read(struct file *filp, char __user *ubuf,
			    size_t count, loff_t *ppos)
{
	size_t done = 0;

	if (mutex_lock_interruptible(&library_lock))
		return -ERESTARTSYS;

	while (done < count && buf_len > 0) {
		size_t chunk = min3(count - done, buf_len,
				    (size_t)(BUF_SIZE - buf_head));
		if (copy_to_user(ubuf + done, event_buf + buf_head, chunk)) {
			mutex_unlock(&library_lock);
			return done ? (ssize_t)done : -EFAULT;
		}
		buf_head = (buf_head + chunk) % BUF_SIZE;
		buf_len -= chunk;
		done += chunk;
	}
	mutex_unlock(&library_lock);
	return done;   /* 0 means no pending events (EOF) */
}

/* Receive one library event from user space. */
static ssize_t library_write(struct file *filp, const char __user *ubuf,
			     size_t count, loff_t *ppos)
{
	char msg[MAX_EVENT + 1];
	size_t len = min_t(size_t, count, MAX_EVENT), i, tail;

	if (count == 0)
		return 0;
	if (copy_from_user(msg, ubuf, len))
		return -EFAULT;
	while (len > 0 && (msg[len - 1] == '\n' || msg[len - 1] == '\0'))
		len--;
	msg[len] = '\0';
	for (i = 0; i < len; i++)           /* keep one event per line */
		if (msg[i] == '\n')
			msg[i] = ' ';

	if (mutex_lock_interruptible(&library_lock))
		return -ERESTARTSYS;

	/* Drop oldest bytes if the new event (+ '\n') does not fit. */
	while (buf_len + len + 1 > BUF_SIZE) {
		buf_head = (buf_head + 1) % BUF_SIZE;
		buf_len--;
	}
	for (i = 0; i < len + 1; i++) {
		tail = (buf_head + buf_len) % BUF_SIZE;
		event_buf[tail] = (i < len) ? msg[i] : '\n';
		buf_len++;
	}
	event_count++;
	mutex_unlock(&library_lock);

	pr_info("library_device: event #%lu: %s\n", event_count, msg);
	return count;
}

static long library_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long value;

	switch (cmd) {
	case LIBRARY_IOC_GET_COUNT:
		mutex_lock(&library_lock);
		value = event_count;
		mutex_unlock(&library_lock);
		return put_user(value, (unsigned long __user *)arg);
	case LIBRARY_IOC_GET_PENDING:
		mutex_lock(&library_lock);
		value = buf_len;
		mutex_unlock(&library_lock);
		return put_user(value, (unsigned long __user *)arg);
	case LIBRARY_IOC_CLEAR:
		mutex_lock(&library_lock);
		buf_head = buf_len = 0;
		event_count = 0;
		mutex_unlock(&library_lock);
		pr_info("library_device: buffer cleared via ioctl\n");
		return 0;
	default:
		return -ENOTTY;
	}
}

static const struct file_operations library_fops = {
	.owner          = THIS_MODULE,
	.open           = library_open,
	.release        = library_release,
	.read           = library_read,
	.write          = library_write,
	.unlocked_ioctl = library_ioctl,
};

/* Make the node world read/writable so the (non-root) backend can use it. */
static char *library_devnode(const struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return NULL;
}

static int __init library_init(void)
{
	int ret;

	event_buf = kzalloc(BUF_SIZE, GFP_KERNEL);
	if (!event_buf)
		return -ENOMEM;

	ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
	if (ret < 0)
		goto err_buf;

	cdev_init(&library_cdev, &library_fops);
	library_cdev.owner = THIS_MODULE;
	ret = cdev_add(&library_cdev, dev_number, 1);
	if (ret < 0)
		goto err_region;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	library_class = class_create(CLASS_NAME);
#else
	library_class = class_create(THIS_MODULE, CLASS_NAME);
#endif
	if (IS_ERR(library_class)) {
		ret = PTR_ERR(library_class);
		goto err_cdev;
	}
	library_class->devnode = library_devnode;

	library_dev = device_create(library_class, NULL, dev_number, NULL,
				    DEVICE_NAME);
	if (IS_ERR(library_dev)) {
		ret = PTR_ERR(library_dev);
		goto err_class;
	}

	pr_info("library_device: loaded, /dev/%s (major %d, minor %d)\n",
		DEVICE_NAME, MAJOR(dev_number), MINOR(dev_number));
	return 0;

err_class:
	class_destroy(library_class);
err_cdev:
	cdev_del(&library_cdev);
err_region:
	unregister_chrdev_region(dev_number, 1);
err_buf:
	kfree(event_buf);
	return ret;
}

static void __exit library_exit(void)
{
	device_destroy(library_class, dev_number);
	class_destroy(library_class);
	cdev_del(&library_cdev);
	unregister_chrdev_region(dev_number, 1);
	kfree(event_buf);
	pr_info("library_device: unloaded (%lu events handled)\n", event_count);
}

module_init(library_init);
module_exit(library_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Smart Library Management System");
MODULE_DESCRIPTION("Character device driver receiving library events from the C++ backend");
MODULE_VERSION("1.0");
