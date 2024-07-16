#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdbool.h>

// for socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> // get peer name 
// daemon 
#include <unistd.h>
// signal
#include <signal.h>

#include "aesdhelper.h"
#include "queue.h"


#define DEBUG_LOG(msg,...) printf("aesdsocket: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("aesdsocket ERROR: " msg "\n" , ##__VA_ARGS__)

#define AESD_PORT "9000"
#define OUT_FILE "/var/tmp/aesdsocketdata"

static bool isTerminated = false;
static bool isAccepted = false;
FILE * fd = NULL;
static void sigHandler(int signal_number) {
    if (signal_number == SIGINT || signal_number == SIGTERM) {
        isTerminated = true;
        if (!isAccepted) {
            closelog();
            if (fd) fclose(fd);
            exit(-1);
        }
    }
}

int main (int argc, char* argv[])
{
    bool isDaemon = false;
    int status;
    int yes = 1;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;  // will point to the results getaddrinfo
    int sockfd; // server socket file desc cripter

    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = sigHandler;
    if (sigaction(SIGTERM, &new_action, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }


    if (argc > 1) {
        if (strcmp("-d", argv[1]) == 0) {
            isDaemon = true;
        }
    }
    if (isDaemon) {
        DEBUG_LOG("aesdsocket starting programm as a deamon");
        pid_t childPid = fork();
        if (childPid > 0) { // parent ID 
            // parent id 
            DEBUG_LOG("Parent exit let child pid: %d running", childPid);
            exit(0);
        } else if (childPid == 0) {// child ID
            DEBUG_LOG("Child daemon process is running\n");
        } else {
            perror("fork");
            exit(1);
        }
    } else {
        DEBUG_LOG("aesdsocket starting normally\n");
    }
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_DEBUG, "Starting aesdsocket isDaemon = %d", isDaemon);
    // TODO: implement socket server
    // 1. Setting the socketaddr 
    // 1.1 One way is to Setting up sockaddr_in and casting to sockaddr
    // 1.2 Another way is to use getaddrinfo()
    
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
    // hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    if ((status = getaddrinfo(NULL, AESD_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status)); // gai_strerror only for error code of getaddrinfo function
        exit(-1);
    }
    // servinfo now points to a linked list of 1 or more struct addrinfos
    // servinfo->ai_addr // type of (struct sockaddr*) is allocated and filled after getaddrinfo

    // 2. Create socket (get the file descriptor)
    #if 0 // FROM ​​Beej's Guide to Network Programming
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    } // End of ​From ​Beej's Guide to Network Programming
    #else 
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC , servinfo->ai_protocol);
    if (sockfd == 0) {
        fprintf(stderr, "could not create socket\n");
        exit (-1);
    } 
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    // 3. Bind the socket 
    if ((status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) != 0) {
        perror("bind");
        fprintf(stderr, "bind socketfd %d error %d\n", sockfd, status);
        close(sockfd);
        exit(-1);
    }
    #endif

    // 4. Connect the socket (donot need because it's server code)
    // if ((status = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) != 0) {
    //     perror("connect");  
    //     fprintf(stderr, "connect socket error %d\n", status);
    //     exit(-1);
    // }
    // printf("Connected socket succesfully sockfd: %d\n", sockfd);   

    // ... do everything until you don't need servinfo anymore ....
    freeaddrinfo(servinfo); // free the linked-list

    // 5. Listen for new connection
    if ((status = listen(sockfd, 10)) == -1) {
        perror("listen");
        fprintf(stderr, "listen socketfd %d error %d\n", sockfd, status);
        close(sockfd);
        exit(-1);
    } 
    DEBUG_LOG("server is now listening to port:%s\n", AESD_PORT);
    fd = fopen(OUT_FILE, "w+"); // open file to read and write
    if (!fd) {
        ERROR_LOG("Cannot open file %s", OUT_FILE);
        close(sockfd);
        closelog(); // close syslog 
        return 1;
    }
    // 6. Accepting the connection 
    int clientfd = 0;
    bool isClientConnected = false;
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen;
    clientAddrLen = sizeof clientAddr;
    char ipstr[INET6_ADDRSTRLEN];
    int port;
    // Mutex for the file accessing
    pthread_mutex_t mutex;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        ERROR_LOG("Cannot Create Mutex");
        close(sockfd);
        closelog(); // close syslog 
        return 1;
    } else {
        DEBUG_LOG("Mutex is inited successfully");
    }
    // Create a list (linked-list) for thread information
    SLIST_HEAD(slisthead, slist_thread_s) head;
    SLIST_INIT(&head);
    while (!isTerminated) {
        isClientConnected = false;
        isAccepted = false;
        // DEBUG_LOG("Main Thread: Waiting to accepting new connection\n");
        clientfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        isAccepted = true;
        if (clientfd != -1) {
            isClientConnected = true;
            getpeername(clientfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
            // deal with both IPv4 and IPv6:
            if (clientAddr.ss_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&clientAddr;
                port = ntohs(s->sin_port);
                inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
            } else { // AF_INET6
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)&clientAddr;
                port = ntohs(s->sin6_port);
                inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
            };
            DEBUG_LOG ("Accepted connection from %d ipaddr: %s\n", clientfd, ipstr);
            syslog(LOG_DEBUG, "Accepted connection from %s", ipstr);
            // start the thread for the new connection 
            pthread_t thread;
            struct thread_data *data = malloc(sizeof(struct thread_data));
            if (!data)
            {
                ERROR_LOG("Failed to malloc thread_data \n");
                return false;
            }
            data->pMutex = &mutex;
            data->isComplete = false;
            data->pFile = fd;
            data->clientFd = clientfd;
            int rc = pthread_create(&thread, NULL, threadfunc, data);
            data->thread = thread;
            if (rc != 0) {
                ERROR_LOG("Failed to create thread for connection from %d ipaddr: %s\n", clientfd, ipstr);
            } else {
                // Add the data to thread list
                slist_thread_t *threaListData = malloc(sizeof(slist_thread_t));
                threaListData->pThreadData = data;
                DEBUG_LOG("Insert new thread %p data to list", data->thread);
                SLIST_INSERT_HEAD(&head, threaListData, entries);
            }
        } else {
            DEBUG_LOG("No client connected\n");
            usleep(100000);
        }
        slist_thread_t *threadListData;
        slist_thread_t *tempThreadListData;
        SLIST_FOREACH_SAFE(threadListData, &head, entries, tempThreadListData) {
            if (threadListData->pThreadData->isComplete) {
                void * thread_rtn = NULL;
                DEBUG_LOG("Thread %p of client %d is completed", threadListData->pThreadData->thread, threadListData->pThreadData->clientFd);
                int tryjoin_rtn =  pthread_join(threadListData->pThreadData->thread, &thread_rtn);
                // int tryjoin_rtn = pthread_tryjoin_np(*threaListData->pThreadData->pThread,&thread_rtn);
                // if( tryjoin_rtn != 0 ) {
                //     tryjoin_rtn = pthread_join(*(threaListData->pThreadData->pThread), &thread_rtn);
                // } 
                if (tryjoin_rtn != 0) {
                    ERROR_LOG("Cannot join the thread %p of client %d", threadListData->pThreadData->thread, threadListData->pThreadData->clientFd);
                } else {
                    DEBUG_LOG("Joined thread %p of client %d successfully", threadListData->pThreadData->thread, threadListData->pThreadData->clientFd);
                    // close client
                    close(threadListData->pThreadData->clientFd);
                    SLIST_REMOVE(&head,threadListData, slist_thread_s, entries);
                    // free pthread data
                    free(threadListData->pThreadData);
                    free(threadListData);
                }
            }
        }
        // fflush(stdin);
    }
    DEBUG_LOG("close server socket");
    close(sockfd);
    closelog(); // close syslog 
    return 0;
}