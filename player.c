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
        perror("Error: gethostname\n");
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
    int left_conn_fd = accept(player_listen_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    int right_fd = open_clientfd(right_addr, right_port);
    int right_conn_fd = accept(player_listen_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);

    // game start
    srand((unsigned int) time(NULL));
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
        r = select(right_conn_fd+1, &rd, NULL, NULL, NULL);
        if(r<0){
            perror("Error: select\n");
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
        nhops = recv_int(in_fd);
        trace_len = recv_int(in_fd);
        if(recv(in_fd, trace, 512*sizeof(int), 0) != 512*sizeof(int)) perror("error recving trace\n");
        if(nhops>0){
            nhops--;
            trace[trace_len] = id;
            trace_len++;
            if(rand()%2){
                out_fd = left_conn_fd;
                out_id = id-1;
            }else{
                out_fd = right_conn_fd;
                out_id = id+1;
            }
            if(out_id == nplayers) out_id = 0;
            else if(out_id < 0) out_id = nplayers-1;
            printf("Sending potato to %d\n", out_id);
            printf("nhops: %d\n", nhops);
            send_int(out_fd, nhops);
            send_int(out_fd, trace_len);
            if(send(out_fd, trace, 512*sizeof(int), 0) != 512*sizeof(int)) perror("error sending trace\n");
        }else{
            if(nhops == -513) break;
            printf("I’m it\n");
            out_fd = master_fd;
            send_int(out_fd, trace_len);
            if(send(out_fd, trace, 512*sizeof(int), 0) != 512*sizeof(int)) perror("error sending trace\n");
            break;
        }
    }
    //freeaddrinfo(master_hints_list);
    close(master_fd);
    close(player_listen_fd);
    close(left_fd);
    close(right_fd);
    return EXIT_SUCCESS;
}