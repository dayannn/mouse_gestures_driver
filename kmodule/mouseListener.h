#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#include "mouseListenerExtern.h"

#define DIR_NAME "mouseListener"
#define NODE_NAME "info"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pakhomov Alexander IU7-71");

static struct proc_dir_entry *procDir;

static int __init procInit( void );
static void __exit procExit( void );

static int nodeOpen(struct inode *inode, struct file *file);
static ssize_t nodeRead(struct file *file, char *buf, size_t count, loff_t *ppos);
static int nodeClose(struct inode *inode, struct file *file);
void makeMessage(void);

static const struct file_operations nodeFops =
{
    .owner = THIS_MODULE,
    .open = nodeOpen,
    .read = nodeRead,
    .release = nodeClose
};

struct mouseInfoItem
{
    int time;
    char dx;
    char dy;
};

struct mouseInfoItem *mouseInfo;
int mouseInfoMaxLen = 500;
int mouseInfoLen = 0;
char *mouseInfoMsg;

module_init(procInit);
module_exit(procExit);
