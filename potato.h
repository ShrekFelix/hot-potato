#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

int establish_socket(
    const char *hostname, 
    const char * port,
    struct addrinfo * p_hints,
    struct addrinfo ** p_hints_list
){
    if (getaddrinfo(hostname, port, p_hints, p_hints_list) != 0) {
        perror("cannot get address info for host:  %s %d\n", hostname, port);
        return EXIT_FAILURE;
    }
    int socket_fd = socket(
        (*p_hints_list)->ai_family, 
        (*p_hints_list)->ai_socktype, 
        (*p_hints_list)->ai_protocol
    );
    if (socket_fd == -1) {
        perror("Error: cannot create socket: %s %d\n", hostname, port);
        return EXIT_FAILURE;
    }

    return socket_fd;
}

int open_clientfd(const char *hostname, const char * port){
    struct addrinfo hints;
    struct addrinfo * hints_list;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int fd = establish_socket(hostname, port, &hints, &hints_list);
    if (connect(fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1){
        perror("Error open clientfd\n");
        return -1;
    }
    return fd;

}

int open_listenfd(const char * port){
    int socket_fd;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    struct addrinfo *hints_list;
    const char *hostname = NULL;
    if (getaddrinfo(hostname, port, &hints, &hints_list) != 0) {
        perror("cannot get address info for host:  %s %d\n", hostname, port);
        return EXIT_FAILURE;
    }
    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1) {
        perror("Error: cannot create socket: %s %d\n", hostname, port);
        return EXIT_FAILURE;
    }
    //socket_fd = establish_socket(hostname, port, &hints, &hints_list);
    int yes = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0){
        perror("Error: setsockopt\n");
        return -1;
    }
    if (bind(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1) {
        perror("Error: cannot bind socket: %s %d\n", hostname, port);
        return -2;
    }
    if (listen(socket_fd, 100) == -1) {
        perror("listen\n"); 
        return -3; 
    }
    return socket_fd;

}

typedef struct _Trace{
    struct _Trace* prev;
    char* id;
} Trace;

typedef struct _Potato{
    int nhops;
    Trace * head;
    Trace * tail;
} Potato;

void send_str(int fd, char * src){
    if(send(fd, src, 512, 0) == -1){
        perror("Error: send\n");
    }
}

void recv_str(int fd, char * des){
    if(recv(fd, des, 512, 0) == -1){
        perror("Error: receive\n");
    }
}

void send_int(int fd, int src){
    if(send(fd, &src, sizeof(int), 0) == -1){
        perror("Error sending %d\n", src);
    }
}

void recv_int(int fd, int* des){
    if(recv(fd, des, sizeof(int), 0) == -1){
        perror("Error receiving\n");
    }
}

