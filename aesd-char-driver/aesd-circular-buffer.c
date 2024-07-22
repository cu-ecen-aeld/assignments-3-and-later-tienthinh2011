/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#define DEBUGG true

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#ifdef DEBUGG 
#include <stdio.h>
#define DEBUG_LOG(msg, ...) printf("circular-buffer DEBUG: " msg "\n", ##__VA_ARGS__)
#define ERROR_LOG(msg, ...) printf("circular-buffer ERROR: " msg "\n", ##__VA_ARGS__)
#else 
#define DEBUG_LOG(msg, ...)
#define ERROR_LOG(msg, ...)
#endif
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    /**
    * TODO: implement per description
    */
    size_t offset = char_offset%AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    DEBUG_LOG("Calculated offset = %lu", offset);
    if (buffer->full) {
        *entry_offset_byte_rtn = offset;
        DEBUG_LOG("Full");
    } else {
        // empty case
        if (buffer->in_offs == 0 || offset > buffer->in_offs) 
        {
            DEBUG_LOG("Empty");
            *entry_offset_byte_rtn = 0;
            return NULL;
        } else {
            DEBUG_LOG("Not empty, not full");
            *entry_offset_byte_rtn = offset;
        }
    }
    return (&buffer->entry[offset]);
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
    buffer->entry[buffer->in_offs].buffptr = add_entry->buffptr;
    buffer->entry[buffer->in_offs].size = add_entry->size;
    DEBUG_LOG("Add new entry at in_offs: %d, %s; out_offs: %d", buffer->in_offs, buffer->entry[buffer->in_offs].buffptr, buffer->out_offs);
    buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    DEBUG_LOG("New in_offs: %d", buffer->in_offs);
    if (buffer->full) {
        // buffer->entry[buffer->in_offs] = add_entry;
        // buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        buffer->out_offs = buffer->in_offs;//(buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        DEBUG_LOG("Buffer is full");
    } else {
        if (buffer->in_offs == buffer->out_offs) {
            buffer->full = true;
            DEBUG_LOG("Buffer started being full");
        }
    }
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
