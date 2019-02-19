#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

int open_clientfd(const char *hostname, const char * port){
    int socket_fd;
    struct addrinfo hints;
    struct addrinfo * hints_list;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, port, &hints, &hints_list) != 0) {
        perror("cannot get address info");
        return -1;
    }
    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1) {
        perror("Error: cannot create socket");
        return -1;
    }
    if (connect(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1){
        perror("Error open clientfd\n");
        return -1;
    }
    return socket_fd;
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
        perror("cannot get address info");
        return -1;
    }
    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1) {
        perror("Error: cannot create socket");
        return -1;
    }
    int yes = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0){
        perror("Error: setsockopt\n");
        return -1;
    }
    if (bind(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1) {
        perror("Error: cannot bind socket: %s %d\n", hostname, port);
        return -1;
    }
    if (listen(socket_fd, 100) == -1) {
        perror("listen\n"); 
        return -1; 
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
    if(send(fd, src, 512, 0) != 512){
        perror("Error: send\n");
    }
}

void recv_str(int fd, char * des){
    if(recv(fd, des, 512, 0) != 512){
        perror("Error: receive\n");
    }
}

void send_int(int fd, int src){
    if(send(fd, &src, sizeof(int), 0) != sizeof(int)){
        perror("Error sending %d\n", src);
    }
}

int recv_int(int fd){
    int i;
    if(recv(fd, &i, sizeof(int), 0) != sizeof(int)){
        perror("Error receiving\n");
    }
    return i;
}