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


#include <stdint.h>
#include <netdb.h>
#include <pthread.h>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>
#include <errno.h>

pthread_mutex_t mutex_state = PTHREAD_MUTEX_INITIALIZER;
namespace EpochLabsTest {
 
}




