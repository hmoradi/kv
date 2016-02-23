#include "server.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>




#include <netdb.h>
#include <pthread.h>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>
#include <errno.h>

pthread_mutex_t mutex_state = PTHREAD_MUTEX_INITIALIZER;
namespace EpochLabsTest {

Server::Server(const std::string& listen_address, int listen_port)
    : listen_fd(-1)
{
    std::cout << "creating server" << std::endl;

    sockaddr_in listen_sockaddr_in;
    std::memset(&listen_sockaddr_in, 0, sizeof(listen_sockaddr_in));
    listen_sockaddr_in.sin_family = AF_INET;
    inet_aton(listen_address.c_str(), &listen_sockaddr_in.sin_addr);
    listen_sockaddr_in.sin_port = htons(listen_port);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) {
        throw_error("could not create socket", errno);
    }

    int t = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t))) {
        throw_error("could not set SO_REUSEADDR", errno);
    }

    if(bind(listen_fd, (struct sockaddr*) &listen_sockaddr_in, sizeof(listen_sockaddr_in))) {
        throw_error("could not bind listen socket", errno);
    }

    if(listen(listen_fd, 48)) {
        throw_error("could not listen on socket", errno);
    }

    //picked up by test_server.py to know server is ready
    //this line must be output after listen returns successfully 
    std::cout << "listening on " << listen_address << ":" << listen_port << std::endl;
}

int Server::accept_new_connection() {
    sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(peer_addr);
    std::memset(&peer_addr, 0, peer_addr_size);
    
    //peer_fd is the file descriptor for the socket of the newly connected client
    int peer_fd = accept4(listen_fd, (struct sockaddr*) &peer_addr, &peer_addr_size, SOCK_CLOEXEC);
  
    if (peer_fd < 0) {
        throw_error("error accepting connection", errno);
    }

    std::cout << "accepted connection, peer_fd=" << peer_fd << std::endl;

    return peer_fd;
}

void Server::run() {
    pthread_t threads[10]; //create 10 handles for threads.
    std::cout << "running ..." << std::endl;
    int clientFD;
    void *arg;
    //replace with your code to implement the run method
    //run() should loop forever servicing requests/connections
    for(;;){
        clientFD = accept_new_connection();
        if (clientFD < 0){
            throw_error("error accepting connection", errno);
        }
        arg = (intptr_t *) clientFD;
        pthread_create(&threads[clientFD], NULL, tcp_server_read, arg);

    }
    //throw_error("Server::run() is not not implemented", 0);
      //  }
}
// int Server::read_from_client (int filedes){
//   char buffer[1024];
//   int nbytes;

//   nbytes = read (filedes, buffer, 1024);
//   if (nbytes < 0){
//       /* Read error. */
//       perror ("read");
//       exit (EXIT_FAILURE);
//   }else if (nbytes == 0){
//     /* End-of-file. */
//     return -1;
//     }else{
//       /* Data read. */
//         std::cout << "read value is " << buffer << std::endl;
//         return 0;
//     }
// }

void Server::throw_error(const char* msg_, int errno_) {
    std::string msg = msg_ + std::string(" errno=") + std::to_string(errno_);
    throw std::runtime_error(msg);
}
void * Server::tcp_server_read(void *arg)
/// This function runs in a thread for every client, and reads incomming data.
/// It also writes the incomming data to all other clients.

{
    int rfd;

    char buf[1024];
    int buflen;
    int wfd;

    rfd = (intptr_t)arg;
    for(;;)
    {
        //read incomming message.
        buflen = read(rfd, buf, sizeof(buf));
        if (buflen < 0){
            /* Read error. */
            perror ("read");
            exit (EXIT_FAILURE);
        }else if (buflen == 0){
            /* End-of-file. */
            return -1;
        }else{
          /* Data read. */
            std::cout << "read value is " << buf << std::endl;
            return 0;
        }

        // send the data to the other connected clients
        pthread_mutex_lock(&mutex_state);

        pthread_mutex_unlock(&mutex_state);

    }
    return NULL;
}
}




