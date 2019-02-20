#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

#define TOSS_PTT {\
    send_int(out_fd, nhops);\
    send_int(out_fd, trace_len);\
    send_trace(out_fd, trace);\
}
#define SHUTDOWN {\
    send_int(out_fd, -100);\
    send_int(out_fd, trace_len);\
    send_trace(out_fd, trace);\
}
#define CATCH_PTT {\
    nhops = recv_int(in_fd);\
    trace_len = recv_int(in_fd);\
    recv_trace(in_fd, trace);\
}


int open_clientfd(const char *hostname, const char * port){
    int socket_fd;
    struct addrinfo hints;
    struct addrinfo * hints_list;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, port, &hints, &hints_list) != 0) {
        perror("getaddrinfo()");
        return -1;
    }
    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1) {
        perror("socket()");
        return -1;
    }
    if (connect(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1){
        perror("connect()\n");
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
        perror("getaddrinfo()");
        return -1;
    }
    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1) {
        perror("socket()");
        return -1;
    }
    int yes = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0){
        perror("setsockopt()");
        return -1;
    }
    if (bind(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1) {
        perror("bind()");
        return -1;
    }
    if (listen(socket_fd, 100) == -1) {
        perror("listen()"); 
        return -1; 
    }
    return socket_fd;
}

void send_str(int fd, char * src){
    if(send(fd, src, 512, MSG_WAITALL) != 512){
        perror("send_str()");
    }
}

void recv_str(int fd, char * des){
    if(recv(fd, des, 512, MSG_WAITALL) != 512){
        perror("recv_str()");
    }
}

void send_int(int fd, int src){
    if(send(fd, &src, sizeof(int), MSG_WAITALL) != sizeof(int)){
        perror("send_int()", src);
    }
}

int recv_int(int fd){
    int i;
    if(recv(fd, &i, sizeof(int), MSG_WAITALL) != sizeof(int)){
        perror("recv_int()");
    }
    return i;
}

void send_trace(int fd, int trace[]){
    if(send(fd, trace, 512*sizeof(int), MSG_WAITALL) != 512*sizeof(int)){
        perror("send_trace()");
    }
}

void recv_trace(int fd, int trace[]){
    if(recv(fd, trace, 512*sizeof(int), MSG_WAITALL) != 512*sizeof(int)){
        perror("recv_trace()");
    }
}