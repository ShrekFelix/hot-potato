#include "potato.h"
#include <time.h>

int main(int argc, char *argv[]){
    // parse input
    if(argc != 4){
        perror("USAGE: ./ringmaster <port_num> <num_players> <num_hops>\n");
        return EXIT_FAILURE;
    }
    const char * portn = argv[1];
    int nplayers = atoi(argv[2]);
    if(nplayers <= 1){
        perror("num_players must be greater than 1.\n");
        return EXIT_FAILURE;
    }
    int nhops = atoi(argv[3]);
    if(nhops<0 || nhops>512){
        perror("num_hops must be greater than or equal to zero and less than or equal to 512.\n");
        return EXIT_FAILURE;
    }
    
    // establish socket
    int socket_fd;
    struct addrinfo hints;
    struct addrinfo *hints_list;
    const char *hostname = NULL;

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

    int yes = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0){
        perror("setsockopt\n");
        return EXIT_FAILURE;
    }

    if (bind(socket_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1) {
        perror("Error: cannot bind socket: %s %d\n", hostname, portn);
        return EXIT_FAILURE;
    }

    if (listen(socket_fd, 100) == -1) {
        perror("listen\n"); 
        return EXIT_FAILURE; 
    }

    printf("Waiting for connection on port\n");

    int client_fds[nplayers];
    int domains[nplayers];
    int types[nplayers];
    int protocols[nplayers];

    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_fd;
    for(int i=0; i<nplayers; i++){
        client_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len); // blocks until connection
        if (client_fd == -1) {
            perror("Error accept\n");
            return EXIT_FAILURE;
        }
        client_fds[i] = client_fd;
        recv_int(client_fd, &domains[i]);
        recv_int(client_fd, &types[i]);
        recv_int(client_fd, &protocols[i]);
        printf("Player %d is ready to play\n", i);
        send_int(client_fd, i);
        send_int(client_fd, nplayers);
    }
    // tell neighbours
    send_int(client_fds[0], client_fds[nplayers-1]);
    send_int(client_fds[0], client_fds[1]);
    for(int i=1; i<nplayers; i++){
        send_int(client_fds[i], client_fds[i-1]);
        send_int(client_fds[i], client_fds[i+1]);
    }

    // initialize potato
    Potato* ptt;
    ptt->head = NULL;
    ptt->tail = NULL;
    ptt->nhops = nhops;
    // send to a random client
    srand( (unsigned int) time(NULL));
    int randn = rand() % nplayers;
    send(client_fds[randn], ptt, sizeof(ptt), 0);

    

    freeaddrinfo(hints_list);
    close(socket_fd);





    
    return EXIT_SUCCESS;
}