#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
// for socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define AESD_PORT "9000"

int main (int argc, char* argv[])
{
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_DEBUG, "Starting aesdsocket");
    // TODO: implement socket server
    // 1. Setting the socketaddr 
    // 1.1 One way is to Setting up sockaddr_in and casting to sockaddr
    // 1.2 Another way is to use getaddrinfo()
    int status;
    int yes = 1;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;  // will point to the results
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
    int sockfd; 
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
    int clientfd;
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen;
    clientAddrLen = sizeof clientAddr;
    clientfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientfd != 0) {
        printf ("Accepted connection from %d\n", clientfd);
    } else {
        printf("No client connected\n");
    }

    // 8. Close the socket 
    printf("close the client socket\n");
    if (clientfd) close(clientfd);
    printf("close server socket");
    close(sockfd);
    closelog(); // close syslog 
    return 0;
}