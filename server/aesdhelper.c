/**
 * @file aesdhelper.c
 * @author Thinh Nguyen (tienthinh2011@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-07-16
 *
 * @copyright Copyright (c) 2024
 *
 */

// Optional: use these functions to add debug or error prints to your application
// #define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg, ...) printf("aesdhelper: " msg "\n", ##__VA_ARGS__)
#define ERROR_LOG(msg, ...) printf("aesdhelper ERROR: " msg "\n", ##__VA_ARGS__)

#include "aesdhelper.h"

#define INIT_BUFFER_SIZE 100

void *threadfunc(void *thread_param)
{
    struct thread_data *data = (struct thread_data *)thread_param;
    int clientFd = data->clientFd;
    FILE* fd = data->pFile; // asumming this file is already open with read/write permission 
    pthread_mutex_t* pMutex = data->pMutex;
    DEBUG_LOG("threadfunc pThread %p for client %d started", data->thread, data->clientFd);

    bool receivedAll = false;
    char *buffer = malloc(INIT_BUFFER_SIZE);
    memset(buffer, 0, INIT_BUFFER_SIZE);
    int currentMaxSize = INIT_BUFFER_SIZE;
    int totalLen = 0;
    int readBytes = 0;

    // Recevie the newline from client
    while (!data->isComplete)
    {
        readBytes = recv(clientFd, &buffer[totalLen], INIT_BUFFER_SIZE, 0);
        if (readBytes >= 0)
        {
            // check last by == \n or not
            totalLen += readBytes;
            if (buffer[totalLen - 1] == '\n')
            {
                DEBUG_LOG("Received full package:\n%s", buffer);
                pthread_mutex_lock(pMutex);
                fseek(fd, 0, SEEK_END);// move pointer to the end of the file
                int ret = fwrite(buffer, 1, totalLen, fd);
                // int ret = fprintf(fd, "%s", buffer);
                if (ret)
                {
                    DEBUG_LOG("Succesfully write %d bytes to file", ret);
                }
                fseek(fd, 0, SEEK_SET);
                memset(buffer, 0, currentMaxSize); // memset the buffer to reuse in read lin
                totalLen = 0;
                // rewind(fd); // move pointer to the beginning of the file
                while (!feof(fd))
                {
                    readBytes = fread(buffer, 1, currentMaxSize, fd);
                    DEBUG_LOG("read %d bytes from file and sending to client", readBytes);
                    send(clientFd, buffer, readBytes, 0);
                }
                DEBUG_LOG("unlock the pMutex");
                pthread_mutex_unlock(pMutex);
                data->isComplete = true;
            }
            if (readBytes == 0)
            {
                DEBUG_LOG("No more reading from client");
                data->isComplete = true;
            }
            // copy the current buffer to new buffer if totalLne is reaching currentMaxSize
            if ((totalLen + INIT_BUFFER_SIZE) > currentMaxSize)
            {
                currentMaxSize *= 2;
                buffer = realloc(buffer, currentMaxSize);// try to use realloc instead below malloc & free
                if (!buffer) {
                    ERROR_LOG("Cannot realloc memory");
                }
                // char *newBuffer = malloc(currentMaxSize); // double current maxSize;
                // memset(newBuffer, 0, currentMaxSize);
                // memcpy(newBuffer, buffer, totalLen);
                // free(buffer);
                // buffer = newBuffer;
            }
        }
    }
    // Return the full content of the file /var/tmp/aesdsocketdata back to client
    if (buffer) free(buffer);


    // // 7. Close the socket 
    // if (clientfd) {
    //     DEBUG_LOG("Closed connection from %s\n", ipstr);
    //     syslog(LOG_DEBUG, "Closed connection from %s", ipstr);
    //     close(clientfd);
    //     isClientConnected = false;
    // } 
}

bool start_thread(pthread_t *thread, pthread_mutex_t *mutex, FILE *file, int clientFd)
{
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if (!data)
    {
        ERROR_LOG("Failed to malloc thread_data \n");
        return false;
    }
    data->pMutex = mutex;
    data->isComplete = false;
    data->pFile = file;
    data->clientFd = clientFd;
    int rc = pthread_create(thread, NULL, threadfunc, data);
    if (rc != 0)
    {
        ERROR_LOG("pthread_create failed with error %d creating thread %p\n", rc, thread);
        return false;
    }
    else
    {
        return true;
    }
}