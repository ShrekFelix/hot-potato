#include <sys/socket.h>
#include <stdlib.h>

struct Trace{
    struct Trace* prev;
    char* id;
};

typedef struct _Potato{
    int nhops;
    struct Trace* tail;
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