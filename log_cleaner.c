#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define BUF_SIZE 256

static char date[BUF_SIZE];
module_param_string(date, date, BUF_SIZE, 0);
MODULE_PARM_DESC(date, "The date to use for cleaning log files");

static int __init clean_logs_init(void) {
    struct file *filp;
    struct inode *inode;
    struct dir_context ctx;
    struct filename fname;
    char buf[BUF_SIZE];
    int error;

    if (strlen(date) == 0) {
        printk(KERN_ERR "Error: date argument is required\n");
        return -EINVAL;
    }

    printk(KERN_INFO "Cleaning log files from date %s\n", date);

    filp = filp_open("/var/log", O_RDONLY, 0);
    if (IS_ERR(filp)) {
        error = PTR_ERR(filp);
        printk(KERN_ERR "Error opening /var/log directory: %d\n", error);
        return error;
    }

    inode = filp->f_path.dentry->d_inode;

    memset(&ctx, 0, sizeof(ctx));
    ctx.pos = 2; // skip . and ..
    ctx.actor = &clean_logs_actor;
    ctx.directory = filp;
    ctx.name = &fname;

    while (!dir_emit(&ctx, " ", 1, 0)) {
        memset(&fname, 0, sizeof(fname));
        error = vfs_readdir(filp, &ctx);
        if (error || !ctx.pos) {
            break;
        }

        if (S_ISREG(inode->i_mode)) {
            memset(&buf, 0, sizeof(buf));
            strncpy_from_user(buf, fname.name, sizeof(buf));
            if (strstr(buf, ".log") != NULL) {
                struct kstat stat;
                error = vfs_stat(fname.name, &stat);
                if (!error) {
                    struct tm timestamp;
                    char date_str[BUF_SIZE];

                    time_to_tm(stat.mtime.tv_sec, 0, &timestamp);
                    sprintf(date_str, "%04d-%02d-%02d", timestamp.tm_year + 1900,
                            timestamp.tm_mon + 1, timestamp.tm_mday);

                    if (strcmp(date_str, date) <= 0) {
                        error = vfs_unlink(inode, &fname);
                        if (error) {
                            printk(KERN_WARNING "Unable to delete file %s: %d\n", fname.name, error);
                        } else {
                            printk(KERN_INFO "Deleted file %s\n", fname.name);
                        }
                    }
                } else {
                    printk(KERN_WARNING "Unable to stat file %s: %d\n", fname.name, error);
                }
            }
        }
    }

    filp_close(filp, NULL);

    return 0;
}

static int clean_logs_actor(void *context, const char *name, const struct stat *stat, u64 ino) {
    return 0;
}

static void __exit clean_logs_exit(void) {
    printk(KERN_INFO "Cleaning log files module unloaded\n");
}

module_init(clean_logs_init);
module_exit(clean_logs_exit);
