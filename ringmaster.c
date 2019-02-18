#include "potato.h"
#include <time.h>
#include <errno.h>

int main(int argc, char *argv[]){
    // parse input
    if(argc != 4){
        perror("USAGE: ./ringmaster <port_num> <num_players> <num_hops>\n");
        return EXIT_FAILURE;
    }
    const char * port = argv[1];
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
    int socket_fd = open_listenfd(port);

    printf("Waiting for connection on port\n");

    int player_fds[nplayers];
    char player_addrs[nplayers][512];
    char player_ports[nplayers][512];

    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int player_fd;
    for(int i=0; i<nplayers; i++){
        player_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len); // blocks until connection
        if (player_fd == -1) {
            printf("%s\n",strerror(errno));
            perror("Error accept\n");
            return EXIT_FAILURE;
        }
        player_fds[i] = player_fd;
        bzero(player_addrs[i], 512);
        bzero(player_ports[i], 512);
        recv_str(player_fd, player_addrs[i]);
        printf("player %d addr: %s\n", i,player_addrs[i]);
        recv_str(player_fd, player_ports[i]);
        printf("player %d ip: %s\n", i,player_ports[i]);
        send_int(player_fd, i);
        send_int(player_fd, nplayers);
        printf("Player %d is ready to play\n", i);
    }
    // tell neighbours
    send_str(player_fds[0], player_addrs[nplayers-1]);
    send_str(player_fds[0], player_ports[nplayers-1]);
    send_str(player_fds[0], player_addrs[1]);
    send_str(player_fds[0], player_ports[1]);
    for(int i=1; i<nplayers-1; i++){
        send_str(player_fds[i], player_addrs[i-1]);
        send_str(player_fds[i], player_ports[i-1]);
        send_str(player_fds[i], player_addrs[i+1]);
        send_str(player_fds[i], player_ports[i+1]);
    }
    send_str(player_fds[nplayers-1], player_addrs[nplayers-2]);
    send_str(player_fds[nplayers-1], player_ports[nplayers-2]);
    send_str(player_fds[nplayers-1], player_addrs[0]);
    send_str(player_fds[nplayers-1], player_ports[0]);
/*
    // initialize potato
    Potato* ptt;
    ptt->head = NULL;
    ptt->tail = NULL;
    ptt->nhops = nhops;
    // send to a random client
    srand( (unsigned int) time(NULL));
    int randn = rand() % nplayers;
    send(player_fds[randn], ptt, sizeof(ptt), 0);    
*/
    //freeaddrinfo(hints_list);
    close(socket_fd);





    
    return EXIT_SUCCESS;
}