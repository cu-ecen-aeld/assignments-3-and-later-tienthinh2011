/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Your Name Here"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */
    struct aesd_dev *dev; /* device information */
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev; /* for other methods */
	return 0;          /* success */
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    struct aesd_dev *dev; /* device information */
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = NULL; /* for other methods */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    
    struct aesd_dev *dev = (struct aesd_dev *)filp->private_data; /* for other methods */
    uint8_t index;
    size_t totalBytes = 0;
    size_t copiedBytes;
    size_t remainingBytes = count;
    size_t failedBytes;

    size_t offset_rtn=0;
    struct aesd_buffer_entry *entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer,
                                                    *f_pos,
                                                    &offset_rtn);
    if (entry == NULL) {
        retval = 0;
        goto read_done;
    }
    do {
        copiedBytes = remainingBytes >= entry->size ? entry->size : remainingBytes;
        remainingBytes -= copiedBytes;
        failedBytes = copy_to_user(&buf[totalBytes], entry->buffptr, copiedBytes);
        if (failedBytes != 0) {
            retval = -EFAULT;
            goto read_done;
        }
        totalBytes += copiedBytes;
        *f_pos+= copiedBytes;
        PDEBUG("read index: %d, totalBytes: %d", index, totalBytes);
        entry += 1;
    } while (entry != &dev->buffer.entry[dev->buffer.in_offs]);

    PDEBUG("read %d bytes", totalBytes);
    retval = totalBytes;

read_done: 
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */
    // copy from user buffer -> new allocated space by kmalloc
    // write to aesd_cicular_buffer which was init during module init
    // if return was not null -> need to free buffer because the buffer was replaced
    char *kbuf = kmalloc(count, GFP_KERNEL);
    size_t numFailedBytes = copy_from_user(kbuf, buf, count);
    if (numFailedBytes != 0) {
        PERROR("copy_from_user failed copied bytes: %d != count: %d", numFailedBytes, count);
        return count - numFailedBytes;
    }
    struct aesd_dev *dev = (struct aesd_dev *)filp->private_data; /* for other methods */
    struct aesd_buffer_entry entry;
    // added when changing return type of aesd_circular_buffer_add_entry to const char*
    const char* rtnptr;
    entry.buffptr = kbuf;
    entry.size = count;
    rtnptr = aesd_circular_buffer_add_entry(&dev->buffer,&entry);
    if (rtnptr != NULL) {
        PDEBUG("Free kbuffer: %s", rtnptr);
        kfree(rtnptr);
    }
    return count;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);
    aesd_circular_buffer_init(&dev->buffer);
    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    struct aesd_buffer_entry *entry;
    uint8_t index;
    AESD_CIRCULAR_BUFFER_FOREACH(entry,&aesd_device.buffer,index) {
        kfree(entry->buffptr);
    }
    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
