#ifndef EPOCHLABS_TEST_SERVER_HPP
#define EPOCHLABS_TEST_SERVER_HPP

#include <string>

namespace EpochLabsTest {

class Server {
public:
    Server(const std::string& listen_address, int listen_port);
    void run();
  
private:
    int listen_fd;
    std::map<std::string, std::string> m;
    //add your members here

    int accept_new_connection();
    void throw_error(const char* msg_, int errno_);
    int server_send(int fd, std::string data);
    //add your methods here
    static void * static_tcp_server_read(void * c);
};

}

#endif
