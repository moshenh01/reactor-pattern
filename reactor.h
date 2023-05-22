#ifndef REACTOR_H
#define REACTOR_H

#include <pthread.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>

// Function pointer type for event handlers
typedef void (*handler_t)(int, void* arg);

typedef struct {
    int listener;               // Listening socket descriptor
    struct pollfd* pfds;        // Array of file descriptors for polling
    int fd_count;               // Current number of file descriptors
    int fd_size;                // Size of the pfds array
    int active;                 // Flag indicating if the reactor is active
    pthread_t thread;           // Thread for reactor
    handler_t *handlers;  // Array of handlers associated with each file descriptor
    char *remoteIP;              // Remote IP address
} Reactor;


typedef Reactor* (*CreateReactorFunc)();
typedef void (*StopReactorFunc)(void* this);
typedef void (*StartReactorFunc)(void* this);
typedef void (*AddFdFunc)(void* this, int fd, handler_t handler);
typedef void (*waitForFunc)(void* this);

void* reactorThread(void* arg);
void handleClientData(int fd, void* arg);
void handleNewConnection(Reactor* reactor);
void* createReactor();
void stopReactor(void* this);
void startReactor(void* this);
void addFd(void* this, int fd, handler_t handler);
void waitFor(void* this);
void* get_in_addr(struct sockaddr *sa);
void deleteReactor(void* this);


int get_listener_socket(void);
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}









#endif /* REACTOR_H */
