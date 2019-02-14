#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>

typedef struct _Trace{
    struct _Trace* prev;
    char* id;
} Trace;

typedef struct _Potato{
    int nhops;
    Trace * head;
    Trace * tail;
} Potato;

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

int establish_socket(const char *hostname, const char * portn){
    int socket_fd;
    struct addrinfo hints;
    struct addrinfo *hints_list;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if (getaddrinfo(hostname, portn, &hints, &hints_list) != 0) {
        perror("cannot get address info for host:  %s %d\n", hostname, portn);
        return EXIT_FAILURE;
    }
    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1) {
        perror("Error: cannot create socket: %s %d\n", hostname, portn);
        return EXIT_FAILURE;
    }
    return socket_fd;
}