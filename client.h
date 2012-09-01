#ifndef __CLIENT__
#define __CLIENT__

void *send_loop(void *socket);
void *recv_loop(void *socket);

pthread_t in_thread, out_thread;

#endif