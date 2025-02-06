#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>       //kmalloc/kfree

static char *filename = "/tmp/module_log.txt";  // Default filename
module_param(filename, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(filename, "The file to write timestamps to. Default: /tmp/module_log.txt");

static int major_number = 0; // Dynamically allocated
static char module_name[] = "mymodule";

// Global variables
static struct file *logfile = NULL;
static ktime_t insertion_time;

// Function prototypes
static int __init mymodule_init(void);
static void __exit mymodule_exit(void);

// Helper function to write to the log file
static int log_timestamp(const char *prefix, ktime_t timestamp) {
  char *buf = NULL;
  struct timespec64 ts;
  int len;
  loff_t pos = 0;

  if (!logfile) {
    printk(KERN_ERR "%s: Log file not initialized.\n", module_name);
    return -ENODEV;
  }

  ts = ktime_to_timespec64(timestamp);
  len = strlen(prefix) + 30; // Enough space for timestamp and newline
  buf = kmalloc(len, GFP_KERNEL);
  if (!buf) {
    printk(KERN_ERR "%s: Failed to allocate memory for log message.\n", module_name);
    return -ENOMEM;
  }

  snprintf(buf, len, "%s: %lld.%09ld\n", prefix, (long long)ts.tv_sec, ts.tv_nsec);
  len = strlen(buf);

  kernel_write(logfile, buf, len, &pos);
  kfree(buf);
  return 0;
}

// Initialization function
static int __init mymodule_init(void) {
  printk(KERN_INFO "%s: Initializing module\n", module_name);

  // Get insertion time
  insertion_time = ktime_get();

  // Open the log file
  logfile = filp_open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (IS_ERR(logfile)) {
    printk(KERN_ERR "%s: Failed to open log file %s: %ld\n", module_name, filename, PTR_ERR(logfile));
    logfile = NULL;
    return PTR_ERR(logfile);
  }

  // Log insertion time
  if (log_timestamp("Module inserted at", insertion_time) != 0) {
      filp_close(logfile, NULL);
      logfile = NULL;
      return -EIO;
  }

  printk(KERN_INFO "%s: Logged insertion time to %s\n", module_name, filename);

  return 0; // Success
}

// Cleanup function
static void __exit mymodule_exit(void) {
  ktime_t removal_time;

  printk(KERN_INFO "%s: Exiting module\n", module_name);

  // Get removal time
  removal_time = ktime_get();

  if (logfile) {
    // Log removal time
    log_timestamp("Module removed at", removal_time);

    // Close the log file
    filp_close(logfile, NULL);
    logfile = NULL;
  } else {
    printk(KERN_WARNING "%s: Log file already closed (or never opened).\n", module_name);
  }

  printk(KERN_INFO "%s: Module unloaded.\n", module_name);
}


module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");