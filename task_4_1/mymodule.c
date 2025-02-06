#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/timekeeping.h>



static struct kobject *time_logger_kobj;
static ktime_t insertion_time;
//static char *filename = "module_time"; // sysfs entry name - not used

// Function to read the time information
static ssize_t time_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct timespec64 ts_ins, ts_rem;
    ktime_t removal_time;
    long long ins_sec, ins_nsec, rem_sec, rem_nsec;
    int len;

    // Get insertion time in timespec format
    ts_ins = ktime_to_timespec64(insertion_time);
    ins_sec = (long long)ts_ins.tv_sec;
    ins_nsec = ts_ins.tv_nsec;

    //Get removal time in timespec format
    removal_time = ktime_get();
    ts_rem = ktime_to_timespec64(removal_time);
    rem_sec = (long long)ts_rem.tv_sec;
    rem_nsec = ts_rem.tv_nsec;

    //Format the string for sysfs
    len = snprintf(buf, PAGE_SIZE, "Inserted: %lld.%09lld\nRemoved: %lld.%09lld\n",
                   ins_sec, ins_nsec, rem_sec, rem_nsec);

    return len;
}

// We don't allow writing to this attribute
static ssize_t time_store(struct kobject *kobj, struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    return -EACCES; // Permission denied
}

//Define the attribute
static struct kobj_attribute time_attribute =
    __ATTR(module_time, 0660, time_show, time_store); // Read-Write attribute

static int __init time_logger_init(void)
{
    int retval;

    // Create the kobject
    time_logger_kobj = kobject_create_and_add("time_logger", kernel_kobj);
    if (!time_logger_kobj)
        return -ENOMEM;

    //Create the sysfs entry
    retval = sysfs_create_file(time_logger_kobj, &time_attribute.attr);
    if (retval)
    {
        kobject_put(time_logger_kobj);
        pr_err("failed to create sysfs file\n");
        return retval;
    }

    //Store the insertion time
    insertion_time = ktime_get();

    pr_info("Time Logger module loaded\n");
    return 0;
}

static void __exit time_logger_exit(void)
{
    kobject_put(time_logger_kobj);
    pr_info("Time Logger module unloaded\n");
}

module_init(time_logger_init);
module_exit(time_logger_exit);

MODULE_LICENSE("GPL");