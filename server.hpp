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
    //add your members here

    int accept_new_connection();
    void throw_error(const char* msg_, int errno_);
    //add your methods here
    void* tcp_server_read (void* arg);
};

}

#endif
