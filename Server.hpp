#ifndef SERVER_HPP
# define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <stdexcept>

class Server {
    public:
        Server(int port, const std::string& password);

    private:
        int port;
        std::string password;

};

#endif