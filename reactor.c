#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "reactor.h"



void* createReactor()
{
    Reactor* reactor = malloc(sizeof(Reactor));
    reactor->fd_count = 0; // For the listener
    reactor->fd_size = 5;
    reactor->pfds = malloc(sizeof(*reactor->pfds) * reactor->fd_size);
    reactor->active = 0;
    reactor->handlers = malloc(sizeof(*reactor->handlers) * reactor->fd_size);
    reactor->listener = 0;
    reactor->remoteIP = malloc(INET6_ADDRSTRLEN);
    return reactor;
}

/// Stop the reactor if it's active
void stopReactor(void* this)
{
    Reactor* reactor = (Reactor*)this;
    if (reactor->active) {
        reactor->active = 0;
        printf("Stopping reactor\n");
        pthread_join(reactor->thread, NULL);
        printf("Reactor stopped\n");
    }
}

// Function that will be called when the reactor is active
// Reactor thread function
//aloop with pool and handle events
void* reactorThread(void* arg)
{
    Reactor* reactor = (Reactor*)arg;
    while (reactor->active) {
        int poll_count = poll(reactor->pfds, reactor->fd_count, 2500);
        //printf("poll_count: %d\n", poll_count);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        // Run through the existing connections looking for data to read
        for (int i = 0; i < reactor->fd_count; i++) {
            // Check if someone's ready to read
            if (reactor->pfds[i].revents & POLLIN) { // We got one!!
                if (reactor->pfds[i].fd == reactor->listener) {
                    // If listener is ready to read, handle new connection
                    reactor->handlers[i](reactor->pfds[i].fd, reactor);
                    break;
                } else {
                    // If not the listener, we're just a regular client
                    reactor->handlers[i](reactor->pfds[i].fd, reactor);
                }
            }
        }
    }

    return NULL;
}

// Start the reactor in a separate thread
void startReactor(void* this)
{
    Reactor* reactor = (Reactor*)this;
    if(reactor->active == 0){
        reactor->active = 1;
    }
    else{
        return;
    }
    int result = pthread_create(&reactor->thread, NULL, reactorThread, reactor);
     if (result != 0) {
        perror("pthread_create");
        exit(1);
    }
   
}

// Add a new file descriptor and its handler to the reactor
void addFd(void* this, int newfd, handler_t handler)
{
    Reactor* reactor = (Reactor*)this;
    if(reactor->listener == 0)
        reactor->listener = newfd;
    
      // Resize the handlers array if necessary
    if (reactor->fd_count >= reactor->fd_size) {
        reactor->fd_size *= 2;  // Double the size
        reactor->handlers = realloc(reactor->handlers, sizeof(handler_t) * reactor->fd_size);
        reactor->pfds = realloc(reactor->pfds, sizeof(*reactor->pfds) * reactor->fd_size);
    }

    reactor->handlers[reactor->fd_count] = handler;
    //printf("Adding fd %d to reactor\n", fd);
    reactor->pfds[reactor->fd_count].fd = newfd;
    reactor->pfds[reactor->fd_count].events = POLLIN; // Check ready-to-read
    reactor->fd_count++;
}



// Wait for the reactor thread to finish
void waitFor(void* this)
{
    Reactor* reactor = (Reactor*)this;

    if (reactor->active) {
       int res = pthread_join(reactor->thread, NULL);
         if(res != 0)
         {
              perror("pthread_join");
              exit(1);
         }
         printf("Reactor thread finished\n");
       
    }
}
//func that deleta all reactor allocated memory
void deleteReactor(void* this)
{
    if(this == NULL)
    {
        printf("Reactor is NULL\n");
        return;
    }
    if(((Reactor*)this)->active)
    {
      waitFor(this);
    }
    printf("Reactor deleted\n");
    Reactor* reactor = (Reactor*)this;
    free(reactor->handlers);
    free(reactor->pfds);
    free(reactor->remoteIP);
    free(reactor);
    
}








