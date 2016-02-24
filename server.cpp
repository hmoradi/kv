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


//added my header files
#include <pthread.h>
#include <stdlib.h>
#define USEDEBUG

#ifdef USEDEBUG
#define Debug( x ) std::cout << x <<std::endl;
#else
#define Debug( x ) 
#endif
volatile fd_set the_state;
pthread_mutex_t mutex_state = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readersQ = PTHREAD_COND_INITIALIZER;
pthread_cond_t writersQ = PTHREAD_COND_INITIALIZER;
namespace EpochLabsTest {
struct readThreadParams {
    Server* server_;
    int client_fd;
};
std::map<std::string,std::string> Server::map_;
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
    readers = 0;
    writers = 0;
    active_writers = 0;
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
    std::cout << "running ..." << std::endl;
    pthread_t threads[10];
    //replace with your code to implement the run method
    //run() should loop forever servicing requests/connections
    //throw_error("Server::run() is not not implemented", 0);
    while(1){
        client_fd = accept_new_connection();
        pthread_mutex_lock(&mutex_state);  // Make sure no 2 threads can create a fd simultanious.
        FD_SET(client_fd, &the_state);  // Add a file descriptor to the FD-set.
        pthread_mutex_unlock(&mutex_state); // End the mutex lock
        struct readThreadParams *params;
        params = (readThreadParams *)malloc(sizeof(*params));
        params->server_ = this;
        params->client_fd = client_fd;
        pthread_create(&threads[client_fd], NULL, Server::createThread, params);
    }
}

void Server::throw_error(const char* msg_, int errno_) {
    std::string msg = msg_ + std::string(" errno=") + std::to_string(errno_);
    throw std::runtime_error(msg);
}
void* Server::createThread(void* arg){
    
    struct readThreadParams *readParams = (readThreadParams *)arg;
    (readParams->server_) -> handleRequest(readParams -> client_fd);
    return NULL;
}
//Extract Commands from socket output
void Server::extractCmnds(char* buf , std::string & truncatedCommand,std::string * lines,int& numberOfCmnds){
    std::string clientRawMessage = std::string(buf);
    std::string lineDelimiter = "\n";
    size_t pos = 0;
    std::string line;
    numberOfCmnds = 0;
    while ((pos = clientRawMessage.find(lineDelimiter)) != std::string::npos) {
        line = clientRawMessage.substr(0, pos);
        if (truncatedCommand.size() >0){
            lines[numberOfCmnds].assign(truncatedCommand + line);
            truncatedCommand.clear();
        }else{
            lines[numberOfCmnds].assign(line);
        }
        
        clientRawMessage.erase(0, pos + lineDelimiter.length());
        numberOfCmnds++;
    }
    if(clientRawMessage.size()>0){
        truncatedCommand += clientRawMessage;    
    }
}

//Parse command line to extract command , key and value
void Server::parseCmnd(std::string cmnd, std::string * cmndContent){
    std::string cmndDelimiter = " ";
    size_t pos = 0;
    std::string token;
    int index = 0;
    while ((pos = cmnd.find(cmndDelimiter)) != std::string::npos) {
        token = cmnd.substr(0, pos);
        cmndContent[index].assign(token);
        cmnd.erase(0, pos + cmndDelimiter.length());
        index++;
    }
    if (cmnd.size()>0){
        cmndContent[index].assign(cmnd);
    }
}
//Process each command based on it's value
void Server::processCmnd(std::string* cmndContent, std::string& response,int rfd){
    if (cmndContent[0].compare("quit") == 0){
        Debug("inside quit");
        if (response.size()>0){
            server_send(rfd,response);
            response.clear();
        }
        quit(rfd);
    }else if(cmndContent[0].compare("set") == 0){
        Debug("inside set");
        response += setMap(cmndContent[1],cmndContent[2]);
    }else if(cmndContent[0].compare("get") == 0){
        response += getMap(cmndContent[1]);
    }else{
        Debug("command is not correct");
    }

}
void * Server::handleRequest(int rfd){
    
    char buf[1024];
    int buflen;
    std::string cmndContent[3];
    std::string cmnds[1024]; //TODO it can be dynamic
    int numberOfCmnds;
    std::string response ;
    std::string truncatedCommand;
    for(;;)
    {
        //read incomming message.
        buflen = read(rfd, buf, sizeof(buf));
        if (buflen <= 0)
        {   
            quit(rfd);
        }else{
            
            extractCmnds(buf,truncatedCommand,cmnds,numberOfCmnds);
            
            Debug("client "<<rfd <<" request has "<<numberOfCmnds<<"number of commands ");
            
            for(int cmndIndex=0;cmndIndex< numberOfCmnds;cmndIndex++){
                parseCmnd(cmnds[cmndIndex],cmndContent);
                processCmnd(cmndContent,response,rfd); 
            }
            if(response.size()>0){
                server_send(rfd,response);
                response.clear();    
            }
        }
        char *begin = &buf[0];
        char *end = begin + sizeof(buf);
        std::fill(begin, end, 0);
    }
    return NULL;
}
std::string Server::setMap(std::string key, std::string val){
    Debug("set called with "<< key << "=" << val);
    pthread_mutex_lock(&mutex_state);
    writers++;
    while (!((readers == 0) && (active_writers == 0))) {
        pthread_cond_wait(&writersQ, &mutex_state);
    }
    active_writers++;
    pthread_mutex_unlock(&mutex_state);

    Server::map_[key]=val;

    pthread_mutex_lock(&mutex_state);
    writers--;
    active_writers--;
    if (writers > 0)
        pthread_cond_signal(&writersQ);
    else
        pthread_cond_broadcast(&readersQ);
    pthread_mutex_unlock(&mutex_state);
    return key + "=" + val + "\n";

}
std::string Server::getMap(std::string key){
    Debug("get called with "<< key);
    std::string val = "null";
     pthread_mutex_lock(&mutex_state);
     while (!(writers == 0))
     pthread_cond_wait(&readersQ, &mutex_state);
     readers++;
     pthread_mutex_unlock(&mutex_state);

     if ( map_.find(key) != map_.end() ) {
        val = map_[key];
    }

     pthread_mutex_lock(&mutex_state);
     if (--readers == 0)
     pthread_cond_signal(&writersQ);
     pthread_mutex_unlock(&mutex_state);
    //pthread_mutex_lock(&mutex_state);
    
    //pthread_mutex_unlock(&mutex_state);
    return key + "=" + val + "\n";
}
int Server::quit(int client_fd){
    Debug("client disconnected. Clearing fd. " << client_fd);
    pthread_mutex_lock(&mutex_state);
    FD_CLR(client_fd, &the_state);      // free fd's from  clients
    pthread_mutex_unlock(&mutex_state);
    close(client_fd);
    pthread_exit(NULL);
}

int Server::server_send(int fd, std::string data){
    int ret;
    Debug("sending message to client "<<fd<<" message is "<<data);
    ret = send(fd, data.c_str(), strlen(data.c_str()),0);
    Debug("len of sent message  "<<ret );
    if(ret <=0){
       throw_error("could not send to cliend",errno);
    }
    return 0;
}
}




