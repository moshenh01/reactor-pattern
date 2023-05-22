
#include "reactor.h"

#define PORT "9034"   // Port we're listening on
Reactor *gReactor;
void sighandler(int sig)
{
   if(sig == SIGINT)
   {
    //printf("Caught signal %d\n", sig);
      stopReactor(gReactor);
      deleteReactor(gReactor);
      exit(0);
   }
}
// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this
    printf("Listening on port 9034\n");

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 5120) == -1) {
        return -1;
    }

    return listener;
}
// Function to handle data from a client
// Remove an index from the set
void del_from_pfds(Reactor* reactor, int fd)
{
    //find i for fd
    int i;
    for (i = 0; i < reactor->fd_count; i++) {
        if (reactor->pfds[i].fd == fd) {
            break;
        }
    }
    if (i == reactor->fd_count) {
        // File descriptor not found
        return;
    }
     //printf("del_from_pfds: %d   i: %d   fd c: %d\n", reactor->pfds[i].fd,i, reactor->fd_count);
    // Copy the one from the end over this one
    reactor->pfds[i] = reactor->pfds[reactor->fd_count - 1];
    reactor->handlers[i] = reactor->handlers[reactor->fd_count - 1];
   
    reactor->fd_count--;
}
// Function to handle a new connection



void handleClientData(int fd, void *arg)
{
    Reactor* reactor = (Reactor*)arg;
    char buf[256]; // Buffer for client data
    int nbytes = recv(fd, buf, sizeof buf, 0);

    if (nbytes <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", fd);
        } else {
            perror("recv");
        }

        close(fd); // Bye!

        del_from_pfds(reactor, fd);
    } else {
             // We got some good data from a client
        printf("pollserver: %s\n", buf);
    }
}
void newConnectionHandler(int fd, void* arg) {
    Reactor* reactor = (Reactor*)arg;
    handleNewConnection(reactor);
}
void handleNewConnection(Reactor* reactor)
{
    int listener = reactor->listener;
    //printf("listener: %d\n", listener);
    //sleep(1);
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    int newfd; // Newly accept()ed socket descriptor

    addrlen = sizeof remoteaddr;
    newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        

            addFd(reactor, newfd,handleClientData);

        printf("pollserver: new connection from %s on socket %d\n",inet_ntop(remoteaddr.ss_family,
        get_in_addr((struct sockaddr*)&remoteaddr),reactor->remoteIP, INET6_ADDRSTRLEN),newfd);
        //printf("ip: %s\n", reactor->remoteIP);
    }

}


// Main function
int main(void)
{
    


    gReactor = createReactor();
    printf("reactor created\n");
    signal(SIGINT, sighandler);

    
    int listener;     // Listening socket descriptor
     // Set up and get a listening socket
    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }
   
    addFd(gReactor, listener, newConnectionHandler);

    startReactor(gReactor);
  
    waitFor(gReactor);
    
    
    

    return 0;
}
