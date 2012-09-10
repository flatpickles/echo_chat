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

void *service_client_thr_func(void *arg)
{
    intptr_t s = (intptr_t)arg;
    int ret;

    char recv_buf[MSG_BUFFER_SIZE];
    char send_buf[MSG_BUFFER_SIZE];

    printf("[%d] new connection\n", (int)s);

    while( (ret = recv(s, recv_buf, MSG_BUFFER_SIZE, 0)) > 0)
    {
        printf("[%d] \"%s\"\n", (int)s, recv_buf);
        sprintf(send_buf, "you sent \"%s\"", recv_buf);
        if( (ret = send(s, send_buf, strlen(send_buf), 0)) <0)
        {
            perror("error on send call");
        }
        memset(recv_buf, 0, MSG_BUFFER_SIZE);
        memset(send_buf, 0, MSG_BUFFER_SIZE);
        fflush(stdout);
    }

    if( ret == 0)
    {
        printf("[%d] connection closed\n", (int)s);
        close(s);
    }
    else if( ret < 0)
    {
        perror("error on recv call");
        perror("connection closed");
        close(s);
    }

    return NULL;
}

int main(int argc, char **argv)
{
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

    if (listen(socket_fd, 10) < 0) {
        perror("listen failure");
        return -1;
    }

    printf("Listening for connections...\n");

    int s = 0;
    struct sockaddr_storage remote_addr;
    socklen_t addr_len;
    pthread_t thread;
    addr_len = sizeof(remote_addr);

    while (1)
    {
        if( (s= accept(socket_fd, (struct sockaddr *)&remote_addr, &addr_len)) < 0)
        {
            perror("error on accept call");
        }
        else
        {
            pthread_create(&thread, NULL, service_client_thr_func, (void *)(intptr_t)s);
        }
    }
}
