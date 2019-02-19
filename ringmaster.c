#include "potato.h"
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
    printf("Potato Ringmaster\nPlayers = %d\nHops = %d\n", nplayers, nhops);
    int master_listen_fd = open_listenfd(port);

    int player_conn_fds[nplayers];
    char player_addrs[nplayers][512];
    char player_ports[nplayers][512];

    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int player_conn_fd;
    for(int i=0; i<nplayers; i++){
        player_conn_fd = accept(master_listen_fd, (struct sockaddr *)&socket_addr, &socket_addr_len); // blocks until connection
        if (player_conn_fd == -1) {
            perror("accept()");
            return EXIT_FAILURE;
        }
        player_conn_fds[i] = player_conn_fd;
        bzero(player_addrs[i], 512);
        bzero(player_ports[i], 512);
        recv_str(player_conn_fd, player_addrs[i]);
        recv_str(player_conn_fd, player_ports[i]);
        send_int(player_conn_fd, i);
        send_int(player_conn_fd, nplayers);
        printf("Player %d is ready to play\n", i);
    }
    // tell neighbours
    send_str(player_conn_fds[0], player_addrs[nplayers-1]);
    send_str(player_conn_fds[0], player_ports[nplayers-1]);
    send_str(player_conn_fds[0], player_addrs[1]);
    send_str(player_conn_fds[0], player_ports[1]);
    for(int i=1; i<nplayers-1; i++){
        send_str(player_conn_fds[i], player_addrs[i-1]);
        send_str(player_conn_fds[i], player_ports[i-1]);
        send_str(player_conn_fds[i], player_addrs[i+1]);
        send_str(player_conn_fds[i], player_ports[i+1]);
    }
    send_str(player_conn_fds[nplayers-1], player_addrs[nplayers-2]);
    send_str(player_conn_fds[nplayers-1], player_ports[nplayers-2]);
    send_str(player_conn_fds[nplayers-1], player_addrs[0]);
    send_str(player_conn_fds[nplayers-1], player_ports[0]);

    int trace_len = 0;
    int trace[512] = {0};
    fd_set rd;
    int r;
    int in_fd;
    int out_fd;
    srand( (unsigned int) time(NULL) + nplayers);

    if(nhops == 0){ // shutdown immediately
        for(int i=0; i<nplayers; ++i){
            out_fd = player_conn_fds[i];
            SHUTDOWN;
        }
        return EXIT_SUCCESS;
    }
    
    int randn = rand() % nplayers;
    printf("Ready to start the game, sending potato to player %d\n", randn);
    out_fd = player_conn_fds[randn];
    TOSS_PTT;
    FD_ZERO(&rd);
    for(int i=0; i<nplayers; ++i){
        FD_SET(player_conn_fds[i], &rd);
    }
    r = select(player_conn_fds[nplayers-1]+1, &rd, NULL, NULL, NULL);
    if(r<0){
        perror("select()");
        return EXIT_FAILURE;
    }
    for(int i=0; i<nplayers; ++i){
        if(FD_ISSET(player_conn_fds[i], &rd)){
            in_fd = player_conn_fds[i];
            CATCH_PTT;
            // trace_len = recv_int(in_fd);
            // recv_trace(in_fd, trace);
        }else{
            // signal others to shutdown
            out_fd = player_conn_fds[i];
            SHUTDOWN;
        }
    }
    printf("Trace of potato:\n");
    for(int i=0; i<trace_len-1; ++i){
        printf("%d,",trace[i]);
    }
    printf("%d\n",trace[trace_len-1]);

    close(master_listen_fd);
    return EXIT_SUCCESS;
}