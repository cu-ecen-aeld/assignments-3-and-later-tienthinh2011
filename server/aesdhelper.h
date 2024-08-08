/**
 * @file aesdhelper.h
 * @author Thinh Nguyen (tienthinh2011@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef AESD_HELPER_H
#define AESD_HELPER_H

#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// for socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> // get peer name 

#include "queue.h"

struct thread_data {
    /* data */
    pthread_t thread; // pointer to thread itself
    pthread_mutex_t *pMutex;
    const char* fileName;
    int clientFd;
    bool isComplete;
};

struct slist_thread_s {
    struct thread_data* pThreadData;
    SLIST_ENTRY(slist_thread_s) entries;
};

typedef struct slist_thread_s slist_thread_t;

void *threadfunc(void *thread_param);

#endif