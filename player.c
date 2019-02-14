#include "potato.h"

int main(int argc, char *argv[]){
    if(argc != 3){
        perror("USAGE: ./player <machine_name> <port_num>.\n");
        return EXIT_FAILURE;
    }
    const char* master_ip = argv[1];
    const char* master_port = argv[2];

    int master_fd;
    struct addrinfo hints;
    struct addrinfo * hints_list;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(master_ip, master_port, &hints, &hints_list) != 0){
        perror("Error: cannot get address info for host\n");
        return EXIT_FAILURE;
    }

    master_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (master_fd == -1){
        perror("Error: cannot create socket\n");
        return EXIT_FAILURE;
    }

    if (connect(master_fd, hints_list->ai_addr, hints_list->ai_addrlen) == -1){
        perror("Error: cannot connect to socket\n");
        return EXIT_FAILURE;
    }

    send_int(master_fd, hints_list->ai_family);
    send_int(master_fd, hints_list->ai_socktype);
    send_int(master_fd, hints_list->ai_protocol);
    int id;
    recv_int(master_fd, &id);
    int nplayers;
    recv_int(master_fd, &nplayers);
    printf("Connected as player %d out of %d total players\n", id, nplayers);

    // ackowledge neighbours
    int left_fd;
    int right_fd;
    recv_int(master_fd, &left_fd);
    recv_int(master_fd, &right_fd);


    // establish server for neighbours
    const char* player_ip = argv[1];
    const char* player_port = argv[2];
    if (getaddrinfo(player_ip, player_port, &hints, &hints_list) != 0){
        perror("Error: cannot get address info for host\n");
        return EXIT_FAILURE;
    }

    master_fd = socket(
        hints_list->ai_family, 
        hints_list->ai_socktype, 
        hints_list->ai_protocol
    );
    if (master_fd == -1){
        perror("Error: cannot create socket\n");
        return EXIT_FAILURE;
    }



    freeaddrinfo(hints_list);
    close(master_fd);




    return EXIT_SUCCESS;
}