#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* data = (struct thread_data *) thread_param;
    data->thread_complete_success = false;
    // print for debug
    printf("Debug: thread data: wait_to_obtain_ms = %d, wait_to_release_ms = %d\n", data->wait_to_obtain_ms, data->wait_to_release_ms);
    // obtain the mutex (mutex which was already init outside of the thread)
    usleep(data->wait_to_obtain_ms*1000);
    int rc = pthread_mutex_lock(data->mutex);
    if (rc != 0) {
        printf("Failed to lock the mutex: %p with error: %d\n", data->mutex, rc);
    } else {
        // printf("Thread obtain the mutex: %ld successfully\n", data->mutex);
        // wait to finish
        usleep(data->wait_to_release_ms*1000);
        rc = pthread_mutex_unlock(data->mutex);
        if (rc != 0) {
            printf("Failed to unlock the mutex:%p \n", data->mutex);
            data->thread_complete_success = false;
        } else {
            // pthread_mutex_destroy(data->mutex);
            data->thread_complete_success = true;
            // printf("threadfunc: successful\n");
        }
    }
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data *data = malloc(sizeof(struct thread_data));
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    int rc = pthread_create(thread, NULL, threadfunc, data);
    if (rc != 0) {
        printf("pthread_create failed with error %d creating thread %p\n",rc, thread);
        return false;
    } else {
        return true;
    }
}

