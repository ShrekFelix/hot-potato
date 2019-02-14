#include "potato.h"
#include <netdb.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if(argc != 3){
        perror("USAGE: ./player <machine_name> <port_num>.\n");
        return EXIT_FAILURE;
    }
    const char* hostname = argv[1];
    const char* portn = argv[2];

    int status;
    int socket_fd;
    struct addrinfo hints;
    struct addrinfo * hints_list;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, portn, &hints, &hints_list);
    if (status != 0){
        perror("Error: cannot get address info for host\n");
        return EXIT_FAILURE;
    }

    socket_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (socket_fd == -1){
        perror("Error: cannot create socket\n");
        return EXIT_FAILURE;
    }

    status = connect(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen);
    if (status == -1){
        perror("Error: cannot connect to socket\n");
        return EXIT_FAILURE;
    }

    send_int(socket_fd, hints_list->ai_family);
    send_int(socket_fd, hints_list->ai_socktype);
    send_int(socket_fd, hints_list->ai_protocol);
    int id;
    recv_int(socket_fd, &id);

    freeaddrinfo(hints_list);
    close(socket_fd);




    return EXIT_SUCCESS;
}