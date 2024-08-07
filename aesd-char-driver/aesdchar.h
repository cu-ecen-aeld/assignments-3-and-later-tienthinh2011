/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#ifdef __KERNEL__
    #include <linux/string.h>
    #ifdef AESD_DEBUG 
     /* This one if debugging is on, and kernel space */
        #define DEBUG_LOG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
        #define ERROR_LOG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
        #define PDEBUG DEBUG_LOG
        #define PERROR ERROR_LOG
    #else
        #define DEBUG_LOG(msg, ...)
        #define ERROR_LOG(msg, ...)
    #endif // DEBUGG
#else // user space #def
    #include <string.h>
    #ifdef AESD_DEBUG 
        #include <stdio.h>
        #define DEBUG_LOG(msg, ...) printf("circular-buffer DEBUG: " msg "\n", ##__VA_ARGS__)
        #define ERROR_LOG(msg, ...) printf("circular-buffer ERROR: " msg "\n", ##__VA_ARGS__)
    #else 
        #define DEBUG_LOG(msg, ...)
        #define ERROR_LOG(msg, ...)
    #endif
#endif // __KERNEL__
#include "aesd-circular-buffer.h"
// maintain courser's provided PDEBUG 
#define PDEBUG DEBUG_LOG
#define PERROR ERROR_LOG

struct aesd_dev
{
    /**
     * TODO: Add structure(s) and locks needed to complete assignment requirements
     */
    struct aesd_circular_buffer buffer;
    struct cdev cdev;     /* Char device structure      */
};


#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */
