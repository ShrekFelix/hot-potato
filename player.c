#include "potato.h"

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
    int player_fd;
    for(int i=1024; i<65535; ++i){
        sprintf(player_port, "%d", i);
        //player_port = itoa(i);
        player_fd = open_listenfd(player_port);
        if(player_fd < 0){
            continue;
        }else{
            break;
        }

    }
    char player_addr[512];
    if(gethostname(player_addr, 512) != 0){
        perror("Error: gethostname\n");
        return EXIT_FAILURE;
    }
    printf("player_addr: %s\n", player_addr);
    printf("player_port: %s\n", player_port);
    send_str(master_fd, player_addr);
    send_str(master_fd, player_port);
    int id;
    recv_int(master_fd, &id);
    int nplayers;
    recv_int(master_fd, &nplayers);
    printf("Connected as player %d out of %d total players\n", id+1, nplayers);

    // ackowledge neighbours
    char left_addr[512];
    char left_ip[512];
    char right_addr[512];
    char right_ip[512];
    recv_str(master_fd, left_addr);
    recv_str(master_fd, left_ip);
    recv_str(master_fd, right_addr);
    recv_str(master_fd, right_ip);
    printf("%s %s\n", left_addr, left_ip);
    printf("%s %s\n", right_addr, right_ip);



    //freeaddrinfo(master_hints_list);
    close(master_fd);




    return EXIT_SUCCESS;
}