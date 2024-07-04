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


#define AESD_PORT "9000"
#define INIT_BUFFER_SIZE 100
#define OUT_FILE "/var/tmp/aesdsocketdata"

static bool isTerminated = false;
static bool isAccepted = false;

static void sigHandler(int signal_number) {
    if (signal_number == SIGINT || signal_number == SIGTERM) {
        isTerminated = true;
        if (!isAccepted) {
            closelog();
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
        printf("aesdsocket starting programm as a deamon\n");
        pid_t childPid = fork();
        if (childPid > 0) { // parent ID 
            // parent id 
            printf("Parent exit let child pid: %d running\n", childPid);
            exit(0);
        } else if (childPid == 0) {// child ID
            printf("Child daemon process is running\n");
        } else {
            perror("fork");
            exit(1);
        }
    } else {
        printf("aesdsocket starting normally\n");
    }
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_DEBUG, "Starting aesdsocket isDaemon = %d", isDaemon);
    // TODO: implement socket server
    // 1. Setting the socketaddr 
    // 1.1 One way is to Setting up sockaddr_in and casting to sockaddr
    // 1.2 Another way is to use getaddrinfo()
    
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
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
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
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
    printf("Listening to port:%s\n", AESD_PORT);

    // 6. Accepting the connection 
    int clientfd = 0;
    bool isClientConnected = false;
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen;
    clientAddrLen = sizeof clientAddr;
    char ipstr[INET6_ADDRSTRLEN];
    int port;
    while (!isTerminated) {
        isClientConnected = false;
        isAccepted = false;
        printf("Waiting to accepting new connection\n");
        clientfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        isAccepted = true;
        if (clientfd != 0) {
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
            printf ("Accepted connection from %d ipaddr: %s\n", clientfd, ipstr);
            syslog(LOG_DEBUG, "Accepted connection from %s", ipstr);
            bool receivedAll = false;
            char *buffer = malloc(INIT_BUFFER_SIZE);
            memset(buffer,0,INIT_BUFFER_SIZE);
            int currentMaxSize = INIT_BUFFER_SIZE;
            int totalLen = 0;
            int readBytes= 0;
            // Recevie the newline from client
            while (1) {
                readBytes = recv(clientfd, &buffer[totalLen], INIT_BUFFER_SIZE, 0);
                if (readBytes >= 0) {
                    // check last by == \n or not
                    totalLen+= readBytes;
                    if (buffer[totalLen-1] == '\n') {
                        printf("Received full package:\n%s", buffer);
                        // Store the newline (append) to /var/tmp/aesdsocketdata
                        FILE * fd = fopen(OUT_FILE, "a+"); // open file to read and write
                        if (fd) {
                            int ret = fprintf(fd, "%s", buffer);
                            if (ret) {
                                printf("Succesfully write %d bytes to file\n", ret );
                            }
                            // Re-read the file from begining and 
                            fclose(fd);
                            fd = fopen(OUT_FILE, "r");
                            if (fd) {
                                memset(buffer,0, currentMaxSize); // memset the buffer to reuse in read lin
                                totalLen = 0;
                                rewind(fd);
                                while (!feof(fd)) {
                                    readBytes = fread(buffer, 1, currentMaxSize, fd);
                                    printf("read %d bytes from %s and sending to client\n", readBytes, OUT_FILE);
                                    send(clientfd, buffer, readBytes, 0);
                                }
                            }
                        } else {
                            perror("fopen");
                        }
                        close(clientfd);
                        break;
                    }
                    if (readBytes == 0) {
                        printf("No more reading from client");
                        close(clientfd);
                        break;
                    }
                    // copy the current buffer to new buffer if totalLne is reaching currentMaxSize
                    if ((totalLen + INIT_BUFFER_SIZE) > currentMaxSize)
                    {
                        currentMaxSize *= 2;
                        char* newBuffer = malloc(currentMaxSize); // double current maxSize;
                        memset(newBuffer, 0, currentMaxSize);
                        memcpy(newBuffer, buffer, totalLen);
                        free(buffer);
                        buffer = newBuffer;
                    }
                }
            }
            // Return the full content of the file /var/tmp/aesdsocketdata back to client 
        } else {
            printf("No client connected\n");
        }
        // 7. Close the socket 
        if (clientfd) {
            printf("Closed connection from %s\n", ipstr);
            syslog(LOG_DEBUG, "Closed connection from %s", ipstr);
            close(clientfd);
            isClientConnected = false;
        } 
    }
    if (clientfd && isClientConnected) close(clientfd); 
    printf("close server socket");
    close(sockfd);
    closelog(); // close syslog 
    return 0;
}