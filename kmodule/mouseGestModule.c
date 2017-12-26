#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#include "mouseGestModuleExtern.h"

#define DIR_NAME "mouseGestModule"
#define NODE_NAME "info"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Muslimov Dayan IU7-73");

static struct proc_dir_entry *procDir;

struct mouseData
{
    char dx;
    char dy;
};

struct mouseData *mousedata;
int mouseDataMaxLen = 500;
int mouseDataLen = 0;
char *mouseDataMsg;


extern bool mouseGestSendCoordinates(char _dx, char _dy)
{
    struct timeval* time;
    printk(KERN_INFO "mouseGestModule: Received coordinates: (%d\t%d)\n", _dx, _dy);

    time = (struct timeval*)kmalloc(sizeof(struct timeval), GFP_KERNEL);
    do_gettimeofday(time);

    mousedata[mouseDataLen].dx = _dx;
    mousedata[mouseDataLen].dy = _dy;

    ++mouseDataLen;
    if(mouseDataLen >= mouseDataMaxLen)
        mouseDataLen = 0;
    kfree(time);
    return 1;
}
EXPORT_SYMBOL(mouseGestSendCoordinates);

void createMessage(void)
{
    int i;
    mouseDataMsg = (char*)kmalloc(1 + mouseDataMaxLen*(2 + sizeof(struct mouseData)), GFP_KERNEL);
    sprintf(mouseDataMsg, "");

    for(i = 0; i < mouseDataLen; ++i)
    {
        sprintf(mouseDataMsg, "%s%d\t%d\n", mouseDataMsg, mousedata[i].dx, mousedata[i].dy);
    }
    mouseDataLen = 0;
}

static int openNode(struct inode* inode, struct file* file)
{
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "mouseGestModule: File opened\n");
    createMessage();
    return 0;
}

static ssize_t readNode(struct file* file, char* buf, size_t count, loff_t* ppos)
{
    int res;
    printk(KERN_INFO "mouseGestModule: reading %d bytes (ppos = %d)\n", count, *ppos);
    if(*ppos >= strlen(mouseDataMsg))
    {
        *ppos = 0;
        printk(KERN_INFO "mouseGestModule: EOF reached");
        return 0;
    }
    if(count > strlen(mouseDataMsg) - *ppos)
        count = strlen(mouseDataMsg) - *ppos;

    res = copy_to_user((void*)buf, mouseDataMsg + *ppos, count);
    *ppos += count;
    printk(KERN_INFO "mouseGestModule: returned %d bytes", count);
    return count;
}

static int closeNode(struct inode* inode, struct file* file)
{
    kfree(mouseDataMsg);
    printk(KERN_INFO "mouseGestModule: File closed\n");
    module_put(THIS_MODULE);
    return 0;
}

static const struct file_operations nodeFops =
{
    .owner = THIS_MODULE,
    .open = openNode,
    .read = readNode,
    .release = closeNode
};


static int __init procInit( void )
{
    struct proc_dir_entry *procNode;
    int ret;

    printk(KERN_INFO "mouseGestModule: mouseGestModule started\n");

    if((procDir = proc_mkdir_mode(DIR_NAME, S_IFDIR | S_IRWXUGO, NULL)) == NULL)
    {
        ret = -ENOENT;
        printk(KERN_INFO "mouseGestModule: Can't create dir /proc/%s\n", DIR_NAME);
        return ret;
    }

    printk(KERN_INFO "mouseGestModule: Directory /proc/%s created", DIR_NAME);

    if((procNode = proc_create_data(NODE_NAME, S_IFREG | S_IRUGO | S_IWUGO,
                                    procDir, &nodeFops, NULL)) == NULL)
    {
        ret = -ENOMEM;
        printk(KERN_INFO "mouseGestModule: Can't create node /proc/%s/%s\n", DIR_NAME, NODE_NAME);
        remove_proc_entry(DIR_NAME, NULL);
        return ret;
    }
	
    mousedata = (struct mouseData*) kmalloc(sizeof(struct mouseData)*mouseDataMaxLen, GFP_KERNEL);

    printk(KERN_INFO "mouseGestModule: Node /proc/%s/%s installed", DIR_NAME, NODE_NAME);
    printk(KERN_INFO "mouseGestModule: Init finished\n");
    return 0;
}


static void __exit procExit( void )
{
    kfree(mouseDataMsg);
    kfree(mousedata);
    remove_proc_entry(NODE_NAME, procDir);
    remove_proc_entry(DIR_NAME, NULL);
}


module_init(procInit);
module_exit(procExit);
