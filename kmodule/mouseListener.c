#include "mouseListener.h"

static int __init procInit( void )
{
    struct proc_dir_entry *procNode;
    int ret;

    printk(KERN_CRIT "Starting mouse listener\n");

    printk(KERN_CRIT "Memory allocated\n");

    if((procDir = proc_mkdir_mode(DIR_NAME, S_IFDIR | S_IRWXUGO, NULL)) == NULL)
    {
        ret = -ENOENT;
        printk(KERN_CRIT "Can't create dir /proc/%s\n", DIR_NAME);
        return ret;
    }

    printk(KERN_CRIT "Directory /proc/%s created", DIR_NAME);

    if((procNode = proc_create_data(NODE_NAME, S_IFREG | S_IRUGO | S_IWUGO,
                                    procDir, &nodeFops, NULL)) == NULL)
    {
        ret = -ENOMEM;
        printk(KERN_CRIT "Can't create node /proc/%s/%s\n", DIR_NAME, NODE_NAME);
        remove_proc_entry(DIR_NAME, NULL);
        return ret;
    }
	
    mouseInfo = (struct mouseInfoItem*)
            kmalloc(sizeof(struct mouseInfoItem)*mouseInfoMaxLen, GFP_KERNEL);

    printk(KERN_CRIT "Node /proc/%s/%s installed", DIR_NAME, NODE_NAME);
    printk(KERN_CRIT "Init finished\n");
    return 0;
}

static void __exit procExit( void )
{
    kfree(mouseInfoMsg);
    kfree(mouseInfo);
    remove_proc_entry(NODE_NAME, procDir);
    remove_proc_entry(DIR_NAME, NULL);
}

extern bool mouseListenerSendCoordinates(char _btns, char _dx, char _dy, char _wheel)
{
    struct timeval* time;
    printk(KERN_CRIT "Received %d (%d, %d), wheel %d\n", _btns, _dx, _dy, _wheel);

    time = (struct timeval*)kmalloc(sizeof(struct timeval), GFP_KERNEL);
    do_gettimeofday(time);

    mouseInfo[mouseInfoLen].time = time->tv_sec;
    mouseInfo[mouseInfoLen].dx = _dx;
    mouseInfo[mouseInfoLen].dy = _dy;

    ++mouseInfoLen;
    if(mouseInfoLen >= mouseInfoMaxLen)
        mouseInfoLen = 0;
    kfree(time);
    return 1;
}
EXPORT_SYMBOL(mouseListenerSendCoordinates);

static int nodeOpen(struct inode* inode, struct file* file)
{
    try_module_get(THIS_MODULE);
    printk(KERN_CRIT "File opened\n");
    makeMessage();
    return 0;
}

void makeMessage(void)
{
    int i;
    mouseInfoMsg = (char*)kmalloc(1 + mouseInfoMaxLen*(5 + sizeof(struct mouseInfoItem)), GFP_KERNEL);
    sprintf(mouseInfoMsg, "");

    for(i = 0; i < mouseInfoLen; ++i)
    {
        sprintf(mouseInfoMsg, "%s%d\t%d\t%d\n", mouseInfoMsg,
                mouseInfo[i].time, mouseInfo[i].dx, mouseInfo[i].dy);
    }
    //printk(KERN_CRIT "buffer = %s\n", mouseInfoMsg);
    mouseInfoLen = 0;
}

static ssize_t nodeRead(struct file* file, char* buf, size_t count, loff_t* ppos)
{
    int res;
    printk(KERN_CRIT "reading %d bytes (ppos = %d)\n", count, *ppos);
    if(*ppos >= strlen(mouseInfoMsg))
    {
        *ppos = 0;
        printk(KERN_CRIT "EOF");
        return 0;
    }
    if(count > strlen(mouseInfoMsg) - *ppos)
        count = strlen(mouseInfoMsg) - *ppos;

    res = copy_to_user((void*)buf, mouseInfoMsg + *ppos, count);
    *ppos += count;
    printk(KERN_CRIT "returned %d bytes", count);
    return count;
}

static int nodeClose(struct inode* inode, struct file* file)
{
    kfree(mouseInfoMsg);
    printk(KERN_CRIT "File closed\n");
    module_put(THIS_MODULE);
    return 0;
}
