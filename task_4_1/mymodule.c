#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/timekeeping.h>
#include <linux/file.h>     
#include <linux/slab.h>    
#include <linux/kernel.h>   

MODULE_LICENSE("GPL");

static struct kobject *time_logger_kobj;
static ktime_t insertion_time;
static ktime_t removal_time;
static char *filename = "module_time_log.txt";
static char *filepath;

static int write_timestamp_to_file(const char *timestamp_string) {
    struct file *file = NULL;
    loff_t pos = 0;
    int len = strlen(timestamp_string);
    int ret = 0;

    file = filp_open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(file)) {
        pr_err("Failed to open file %s: %ld\n", filepath, PTR_ERR(file));
        return PTR_ERR(file);
    }

    ret = kernel_write(file, timestamp_string, len, &pos);
    if (ret != len) {
        pr_err("Failed to write to file %s\n", filepath);
    }

    filp_close(file, NULL);
    return 0;
}

static ssize_t time_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct timespec64 ts_ins, ts_rem;
    long long ins_sec, ins_nsec, rem_sec, rem_nsec;
    int len;

    ts_ins = ktime_to_timespec64(insertion_time);
    ins_sec = (long long)ts_ins.tv_sec;
    ins_nsec = ts_ins.tv_nsec;

    ts_rem = ktime_to_timespec64(removal_time);
    rem_sec = (long long)ts_rem.tv_sec;
    rem_nsec = ts_rem.tv_nsec;

    len = snprintf(buf, PAGE_SIZE, "Inserted: %lld.%09lld\nRemoved: %lld.%09lld\n",
                   ins_sec, ins_nsec, rem_sec, rem_nsec);

    return len;
}

static ssize_t time_store(struct kobject *kobj, struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    return -EACCES;
}

static struct kobj_attribute time_attribute =
    __ATTR(module_time, 0660, time_show, time_store);

static int __init time_logger_init(void)
{
    int retval;
    char ins_time_str[128];
    struct timespec64 ts_ins;

    filepath = kmalloc(strlen(filename) + 7, GFP_KERNEL);
    if (!filepath)
        return -ENOMEM;
    snprintf(filepath, strlen(filename) + 7, "/tmp/%s", filename);

    time_logger_kobj = kobject_create_and_add("time_logger", kernel_kobj);
    if (!time_logger_kobj)
        return -ENOMEM;

    retval = sysfs_create_file(time_logger_kobj, &time_attribute.attr);
    if (retval)
    {
        kobject_put(time_logger_kobj);
        pr_err("failed to create sysfs file\n");
        return retval;
    }

    insertion_time = ktime_get();

    ts_ins = ktime_to_timespec64(insertion_time);
    snprintf(ins_time_str, sizeof(ins_time_str), "Module inserted at: %lld.%09lld\n",
             (long long)ts_ins.tv_sec, ts_ins.tv_nsec);

    write_timestamp_to_file(ins_time_str);

    pr_info("Time Logger module loaded\n");
    return 0;
}

static void __exit time_logger_exit(void)
{
    char rem_time_str[128];
    struct timespec64 ts_rem;

    removal_time = ktime_get();

    ts_rem = ktime_to_timespec64(removal_time);
    snprintf(rem_time_str, sizeof(rem_time_str), "Module removed at: %lld.%09lld\n",
             (long long)ts_rem.tv_sec, ts_rem.tv_nsec);

    write_timestamp_to_file(rem_time_str);

    kobject_put(time_logger_kobj);
    kfree(filepath);
    pr_info("Time Logger module unloaded\n");
}
module_init(time_logger_init);
module_exit(time_logger_exit);