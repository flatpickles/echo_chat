#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_BUFFER_SIZE 256

fd_set fd_list;

int main(int argc, char **argv) {
    // check arguments
    if (argc != 2) {
        printf("usage: server listening_port\n");
        return 0;
    }
    char *port_str = argv[1];

    // set up listening socket

    int rv, socket_fd;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    if ((rv = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
        fprintf(stderr, "\n getaddrinfo error: %s", gai_strerror(rv));
        return -1;
    }

    if ((socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0) {
        perror("\n Failed to create socket");
        return -1;
    }

    if (bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
        close(socket_fd);
        fprintf(stderr, "server error: bind failure.\n");
        return -1;
    }

    freeaddrinfo(servinfo);

    if (listen(socket_fd, 10) < 0) {
        perror("listen failure");
        return -1;
    }

    printf("Listening for connections...\n");

    // set up select + vars for accepting connections

    fd_set fd_list_temp;
    int fd_max, new_socket;
    struct sockaddr_storage remote_addr;
    socklen_t addr_len;
    FD_ZERO(&fd_list);
    FD_ZERO(&fd_list_temp);
    fd_max = socket_fd;
    FD_SET(socket_fd, &fd_list);

    // run select loop

    while (1) {
        fd_list_temp = fd_list;

        if (select(fd_max + 1, &fd_list_temp, NULL, NULL, NULL) == -1) {
            perror("error on select call");
            return -1;
        }

        int i;
        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &fd_list_temp)) {
                if (i == socket_fd) {
                    // new connection
                    addr_len = sizeof(remote_addr);
                    new_socket = accept(socket_fd, (struct sockaddr*)&remote_addr, &addr_len);
                    if (new_socket < 0) {
                        perror("error on accept call");
                    } else {
                        FD_SET(new_socket, &fd_list);
                        fd_max = fd_max < new_socket ? new_socket : fd_max;
                        printf("[%d] new connection\n", new_socket);
                    }
                } else {
                    // recv from client
                    int rv;
                    char sent[MSG_BUFFER_SIZE];
                    if ((rv = recv(i, sent, MSG_BUFFER_SIZE, 0)) <= 0) {
                        if (rv != 0) perror("error on recv call");
                        close(i);
                        FD_CLR(i, &fd_list);
                        printf("[%d] connection closed\n", i);
                        continue;
                    }

                    // print and echo
                    printf("[%d] \"%s\"\n", i, sent);
                    char reply[MSG_BUFFER_SIZE];
                    sprintf(reply, "you sent \"%s\"", sent);
                    if ((rv = send(i, reply, strlen(reply), 0)) < 0) {
                        perror("error on send call");
                    }
                }
            }
            fflush(stdout);
        }
    }
}
