#include "potato.h"
#include <assert.h>

int main(int argc, char *argv[]){
    if(argc != 3){
        perror("USAGE: ./player <machine_name> <port_num>.\n");
        return EXIT_FAILURE;
    }
    const char* master_addr = argv[1];
    const char* master_port = argv[2];
    int master_fd = open_clientfd(master_addr, master_port);
    // player socket
    char *player_port;
    int player_listen_fd;
    for(int i=8000; i<9999; ++i){
        sprintf(player_port, "%d", i);
        //player_port = itoa(i);
        player_listen_fd = open_listenfd(player_port);
        if(player_listen_fd < 0){
            continue;
        }else{
            break;
        }
    }
    if(player_listen_fd<0){
        perror("no available port\n");
        return EXIT_FAILURE;
    }
    char player_addr[512];
    if(gethostname(player_addr, 512) != 0){
        perror("gethostname()\n");
        return EXIT_FAILURE;
    }
    send_str(master_fd, player_addr);
    send_str(master_fd, player_port);
    int id = recv_int(master_fd);
    int nplayers = recv_int(master_fd);
    printf("Connected as player %d out of %d total players\n", id, nplayers);

    // ackowledge neighbours
    char left_addr[512];
    char left_port[512];
    char right_addr[512];
    char right_port[512];
    recv_str(master_fd, left_addr);
    recv_str(master_fd, left_port);
    recv_str(master_fd, right_addr);
    recv_str(master_fd, right_port);

    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int left_fd = open_clientfd(left_addr, left_port);
    int right_conn_fd = accept(player_listen_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    int right_fd = open_clientfd(right_addr, right_port);
    int left_conn_fd = accept(player_listen_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);

    // game start
    srand((unsigned int) time(NULL) + id);
    int trace[512];
    int trace_len = 0;
    int nhops;
    fd_set rd;
    int r;
    int in_fd;
    int out_fd;
    int out_id;
    while(1){
        FD_ZERO(&rd);
        FD_SET(master_fd, &rd);
        FD_SET(left_fd, &rd);
        FD_SET(right_fd, &rd);
        r = select(left_conn_fd+1, &rd, NULL, NULL, NULL);
        if(r<0){
            perror("select()");
            return EXIT_FAILURE;
        }
        if(FD_ISSET(master_fd, &rd)){
            in_fd = master_fd; 
        }else if(FD_ISSET(left_fd, &rd)){
            in_fd = left_fd;
        }else{
            assert(FD_ISSET(right_fd, &rd));
            in_fd = right_fd;
        }
        CATCH_PTT;
        nhops--;
        trace[trace_len] = id;
        trace_len++;
        if(nhops>0){
            // randomly choose a neigbour
            if(rand()%2){
                out_fd = left_conn_fd;
                out_id = id-1;
            }else{
                out_fd = right_conn_fd;
                out_id = id+1;
            }
            // correct neigbour id
            if(out_id == nplayers) out_id = 0;
            else if(out_id < 0) out_id = nplayers-1;
            // send potato to it
            printf("Sending potato to %d\n", out_id);
            TOSS_PTT;
        }else{
            if(nhops <= -100 ) break; // game already over; this is the shutdown signal from the master
            // game over
            printf("I’m it\n");
            out_fd = master_fd;
            TOSS_PTT;
            break;
        }
    }
    close(master_fd);
    close(player_listen_fd);
    close(left_fd);
    close(right_fd);
    return EXIT_SUCCESS;
}