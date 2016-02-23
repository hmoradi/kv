#ifndef EPOCHLABS_TEST_SERVER_HPP
#define EPOCHLABS_TEST_SERVER_HPP

#include <string>
#include <map>
namespace EpochLabsTest {

class Server {
public:
    Server(const std::string& listen_address, int listen_port);
    void run();
  
private:
    int listen_fd;
    //add your members here
    int client_fd;
    static std::map<std::string,std::string> map_;
    int accept_new_connection();
    void throw_error(const char* msg_, int errno_);
    //add your methods here
    static void* createThread(void* arg);
    void * handleRequest(int arg);
    int server_send(int fd, std::string data);
    int quit(int fd);
    std::string do_command(std::string key, std::string val);
};

}

#endif
