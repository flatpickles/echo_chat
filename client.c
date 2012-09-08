#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_BUFFER_SIZE 256

void *send_loop(void *socket);
void *recv_loop(void *socket);

pthread_t in_thread, out_thread;

void *send_loop(void *socket) {
    int rv, socket_fd = (int)socket;
    while (1) {
        char input[MSG_BUFFER_SIZE];
        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strlen(input)-1] = 0; // remove newline
            if (!strcmp(input, "quit")) {
                exit(0);
            }
            if ((rv = send(socket_fd, input, strlen(input)*4, 0)) < 0) {
                perror("socket failure during send");
                exit(-1);
            }
        }
        memset(input, 0, sizeof(input));
        fflush(stdout);
    }
    return NULL;
}

void *recv_loop(void *socket) {
    int rv, socket_fd = (int)socket;
    while (1) {
        char sent[MSG_BUFFER_SIZE];
        if ((rv = recv(socket_fd, sent, MSG_BUFFER_SIZE, 0)) == -1) {
            perror("socket failure during recv");
            exit(-1);
        }
        if (rv == 0) {
            printf("[server has closed the connection]\n");
            exit(0);
        }
        printf("[server] %s\n", sent);
        memset(sent, 0, sizeof(sent));
        fflush(stdout);

        /**
        // memory leak for valgrind example...
        malloc(256);
        **/
    }
    return NULL;
}

int main(int argc, char **argv) {
    // check arguments
    if (argc != 3) {
        printf("usage: client server_hostname port\n");
        return 0;
    }
    char *ip_str = argv[1];
    char *port_str = argv[2];
    printf("Connecting with %s on port %s...\n", ip_str, port_str);

    /**
    // segfault for GDB example...
    printf("%s\n", (char *)0x0030);
    **/

    // establish the connection

    int rv, socket_fd;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    if ((rv = getaddrinfo(ip_str, port_str, &hints, &servinfo)) != 0) {
        fprintf(stderr, "\n getaddrinfo error: %s", gai_strerror(rv));
        return -1;
    }

    if ((socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        perror("failed to create socket");
        return -1;
    }

    int ret;
    if ((ret = connect(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
        perror("failed to connect with server");
        close(socket_fd);
        return -1;
    }

    freeaddrinfo(servinfo);
    printf("Connection established!\n");

    // start the threads

    pthread_create(&in_thread, NULL, send_loop, (void*)socket_fd);
    pthread_create(&out_thread, NULL, recv_loop, (void*)socket_fd);
    pthread_join(in_thread, NULL);
    pthread_join(out_thread, NULL);

    return 0;
}
